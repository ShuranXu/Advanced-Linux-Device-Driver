# Assignment #1: blinkplus driver implementation

## Program Description

The program basically patches work of the virtual keyboard's `ioctl()` to the dedicated work queue upon the expiration of the registered timer, which reactivates itself repeatly upon expiration. User programs are free to set keyboard LED control values to be delivered to the virtual keyboard's `ioctl()`. 

`blink_ioctl()` is implemented to support the following commands:

    * SETLED: set the LED value
    * SETINV: set the timer interval value
    * GETINV: get the timer interval value

In the timer callback function `my_timer_func()`, `do_work()`, which is essentially the function to launch the virtual keyboard's `ioctl()`, is registered to be executed by the dedicated work queue via `queue_work()`. In addition, the timer's `expires` variable is updated and `add_timer()` is called to reactivate the timer. 

