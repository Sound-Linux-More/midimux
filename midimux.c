#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <alsa/asoundlib.h>

// function definitions
int open_seq(snd_seq_t **seq_handle, int *mux, int dev[], int devices);
void midi_multiplex(snd_seq_t *seq_handle, int mux, int dev[], int devices);
int note_event(snd_seq_event_t *ev);
int cc_event(snd_seq_event_t *ev);
int find_port(int port, int dev[], int devices);


// open the sequencer and create all the ports
int open_seq(snd_seq_t **seq_handle, int *mux, int dev[], int devnum)
{

    char portname[64];
    char error[64];

    // opening the sequencer
    if (snd_seq_open(seq_handle, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0)
    {
        fprintf(stderr, "Error opening ALSA sequencer.\n");
        return(-1);
    }
    snd_seq_set_client_name(*seq_handle, "midimux");
    int in_out_opts = SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE|
                      SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ;

    // creating final multiplexer port
    printf("creating port: MUX\n");
    if ((*mux = snd_seq_create_simple_port(*seq_handle, "MUX",
                                           in_out_opts, SND_SEQ_PORT_TYPE_APPLICATION)) < 0)
    {
        fprintf(stderr, "Error creating MUX port.\n");
        return(-1);
    }

    // creating n device ports
    for (int i = 0; i < devnum; i++)
    {
        sprintf(portname, "DEV%02d", i);
        printf("creating port: %s\n", portname);
        if ((dev[i] = snd_seq_create_simple_port(*seq_handle, portname,
                      in_out_opts, SND_SEQ_PORT_TYPE_APPLICATION)) < 0)
        {
            sprintf(error, "Error creating DEV%02d\n", i);
            fprintf(stderr, error);
            return(-1);
        }
    }
    // all went OK!
    return(0);
}


int note_event(snd_seq_event_t *ev)
{
    return ((ev->type == SND_SEQ_EVENT_NOTEON)
            || (ev->type == SND_SEQ_EVENT_NOTEOFF));
}


int cc_event(snd_seq_event_t *ev)
{
    return (ev->type == SND_SEQ_EVENT_CONTROLLER);
}

int find_port(int port, int dev[], int devices)
{
    for(int i=0; i<devices; i++)
    {
        if(dev[i] == port)
        {
            return i;
        }
    }
    // this should never happen
    return -1;
}


void midi_multiplex(snd_seq_t *seq_handle, int mux, int dev[], int devices)
{
    snd_seq_event_t *ev;
    int entrance;
    int channel;
    int device;

    do
    {
        snd_seq_event_input(seq_handle, &ev);
        entrance = ev->dest.port;
        snd_seq_ev_set_subs(ev);
        snd_seq_ev_set_direct(ev);

        // MUX -> DEV routing
        if (entrance == mux)
        {
            // if message is note-on, note-off, or cc
            // route it by channel number
            if(note_event(ev))
            {
                channel = ev->data.note.channel;
                if (channel < devices)
                {
                    ev->data.note.channel = 0;
                    snd_seq_ev_set_source(ev, dev[channel]);
                    snd_seq_event_output_direct(seq_handle, ev);
                }
            }
            else if(cc_event(ev))
            {
                channel = ev->data.control.channel;
                if (channel < devices)
                {
                    ev->data.control.channel = 0;
                    snd_seq_ev_set_source(ev, dev[channel]);
                    snd_seq_event_output_direct(seq_handle, ev);
                }
            }
            else
            {
                // otherwise send the received message on all
                // the device ports
                for(int devnum=0; devnum<devices; devnum++)
                {
                    snd_seq_ev_set_source(ev, dev[devnum]);
                    snd_seq_event_output_direct(seq_handle, ev);
                }
            }
        }
        else
        {
            // DEV -> MUX routing
            device = find_port(entrance, dev, devices);
            if (note_event(ev))
            {
                ev->data.note.channel = device;
            }
            else if (cc_event(ev))
            {
                ev->data.control.channel = device;
            }
            snd_seq_ev_set_source(ev, mux);
            snd_seq_event_output_direct(seq_handle, ev);
        }

        snd_seq_free_event(ev);
    }
    while (snd_seq_event_input_pending(seq_handle, 0) > 0);
}


int main(int argc, char *argv[])
{
    snd_seq_t *seq_handle;
    struct pollfd *pfd;
    int npfd;

    int devnum = 2;
    int dev[devnum];
    int mux;

    // device count from command line
    // two devices minimum
    if (argc > 1)
    {
        devnum = atoi(argv[1]);
        if (devnum < 2)
        {
            devnum = 2;
        }
    }

    if (open_seq(&seq_handle, &mux, dev, devnum) < 0)
    {
        fprintf(stderr, "Error opening the sequencer and ports\n");
        exit(1);
    }
    npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
    pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
    snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);
    while (1)
    {
        if (poll(pfd, npfd, 100000) > 0)
        {
            midi_multiplex(seq_handle, mux, dev, devnum);
        }
    }
}
