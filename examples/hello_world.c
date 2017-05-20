#include <stdio.h>
#include "cam.h"

#define LOOPID 1010

int main()
{

    /* Initialise tracers */
    CAM_init (CAM_MEMORY_PROFILE);
    CAM_init (CAM_LOOP_PROFILE);

    int A[1000];
    int B[2000];
    int C[1001];

    CAM_profileLoopInvocationStart(LOOPID);
    for(int i = 0; i < 1000; i++)
    {
        CAM_profileLoopIterationStart();

        /* Read B */
        CAM_profileLoopSeenInstruction(1);
        CAM_mem(1, (uintptr_t)(&(B[i*2])), sizeof(int), 0, 0, 0, 0);

        /* Write A */
        CAM_profileLoopSeenInstruction(2);
        CAM_mem(2, 0, 0, 0, 0, (uintptr_t)(&(A[i])), sizeof(int));

        /* Read C */
        CAM_profileLoopSeenInstruction(3);
        CAM_mem(3, (uintptr_t)(&(C[i+1])), sizeof(int), 0, 0, 0, 0);

        /* Write C */
        CAM_profileLoopSeenInstruction(4);
        CAM_mem(4, 0, 0, 0, 0, (uintptr_t)(&(C[i])), sizeof(int));

        /* Application code */
        A[i] = B[i*2];
        C[i] = C[i+1];
    }
    CAM_profileLoopInvocationEnd();


    /* Shutdown tracers */
    CAM_shutdown (CAM_MEMORY_PROFILE);
    CAM_shutdown (CAM_LOOP_PROFILE);
}
