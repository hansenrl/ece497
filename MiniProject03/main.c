/*
 *   main.c
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2005
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

// Standard Linux headers
#include     <stdio.h>	// Always include this header
#include     <stdlib.h>	// Always include this header
#include     <signal.h>	// Defines signal-handling functions (i.e. trap Ctrl-C)
#include     <unistd.h>

// Application headers
#include     "debug.h"
#include     "video_thread.h"
#include     "audio_thread.h"
#include     "thread.h"

pthread_t audioThread, videoThread;
void * audioThreadReturn, * videoThreadReturn;

/* Global thread environments */
video_thread_env video_env = {0};
audio_thread_env audio_env = {0};

/* Store previous signal handler and call it */
void (*pSigPrev)(int sig);

/* Callback called when SIGINT is sent to the process (Ctrl-C) */
void signal_handler(int sig) {
    DBG( "Ctrl-C pressed, cleaning up and exiting..\n" );

    video_env.quit = 1;
#ifdef _DEBUG_
    sleep(1);
#endif
    audio_env.quit = 1;

    if( pSigPrev != NULL )
        (*pSigPrev)( sig );
}

//*****************************************************************************
//*  main
//*****************************************************************************
int main(int argc, char *argv[])
{
/* The levels of initialization for initMask */
#define VIDEOTHREADATTRSCREATED 0x1
#define VIDEOTHREADCREATED      0x2
#define AUDIOTHREADATTRSCREATED 0x4
#define AUDIOTHREADCREATED      0x8
    unsigned int    initMask  = 0;
    int             status    = EXIT_SUCCESS;

    void *videoThreadReturn;

    /* Set the signal callback for Ctrl-C */
    pSigPrev = signal( SIGINT, signal_handler );

    /* Make video frame buffer visible */
    system("./vid3Show");

    /* Create a thread for video */
    DBG( "Creating video thread\n" );
    printf( "\tPress Ctrl-C to exit\n" );

    /* Create a thread for video loopthru */
    if(launch_pthread(&videoThread, TIMESLICE, 0, video_thread_fxn, &video_env) != thread_SUCCESS){
	ERR("pthread create failed for video\n");
	status = EXIT_FAILURE;
	goto cleanup;
    }
    initMask |= VIDEOTHREADCREATED;
    //videoThreadReturn = video_thread_fxn( (void *) &video_env );

    /* Create a thread for audio loopthru */
    if(launch_pthread(&audioThread, REALTIME, 99, audio_thread_fxn, &audio_env) != thread_SUCCESS){
	ERR("pthread create failed for video\n");
	status = EXIT_FAILURE;
	goto cleanup;
    }
    initMask |= AUDIOTHREADCREATED;

cleanup:
    pthread_join(videoThread, (void **) &videoThreadReturn);
    pthread_join(audioThread, (void **) &audioThreadReturn);
    /* Make video frame buffer invisible */
    system("./resetVideo");

    exit( status );
}
