FreeRTOS-Kernel is pinned by `freertos.tag` and cloned into `FreeRTOS-Kernel/`
by `tools/fetch-deps.sh`. The only local change is `tools/freertos-timer0.patch`,
which lets `configAVR_USE_TIMER0_TICK` select the system tick.
