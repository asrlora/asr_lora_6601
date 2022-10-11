/*!
 * \file      timer.c
 *
 * \brief     Timer objects and scheduling management implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#include "utilities.h"
#include "timer.h"
#include "rtc-board.h"
#include "sx126x-board.h"

/*!
 * Safely execute call back
 */
#define ExecuteCallBack( _callback_, context ) \
    do                                         \
    {                                          \
        if( _callback_ == NULL )               \
        {                                      \
            while( 1 );                        \
        }                                      \
        else                                   \
        {                                      \
            _callback_( context );             \
        }                                      \
    }while( 0 );
volatile uint8_t HasLoopedThroughMain = 0;
static TimerTime_t g_systime_ref = 0;

/*!
 * Timers list head pointer
 */
static TimerEvent_t *TimerListHead = NULL;

/*!
 * \brief Adds or replace the head timer of the list.
 *
 * \remark The list is automatically sorted. The list head always contains the
 *         next timer to expire.
 *
 * \param [IN]  obj Timer object to be become the new head
 * \param [IN]  remainingTime Remaining time of the previous head to be replaced
 */
static void TimerInsertNewHeadTimer( TimerEvent_t *obj );

/*!
 * \brief Adds a timer to the list.
 *
 * \remark The list is automatically sorted. The list head always contains the
 *         next timer to expire.
 *
 * \param [IN]  obj Timer object to be added to the list
 * \param [IN]  remainingTime Remaining time of the running head after which the object may be added
 */
static void TimerInsertTimer( TimerEvent_t *obj );

/*!
 * \brief Sets a timeout with the duration "timestamp"
 *
 * \param [IN] timestamp Delay duration
 */
static void TimerSetTimeout( TimerEvent_t *obj );

/*!
 * \brief Check if the Object to be added is not already in the list
 *
 * \param [IN] timestamp Delay duration
 * \retval true (the object is already in the list) or false
 */
static bool TimerExists( TimerEvent_t *obj );


void TimerSetSysTime( TimerSysTime_t sysTime )
{
    TimerTime_t cur_time = RtcGetTimerValue( );
    TimerTime_t set_time = (TimerTime_t)sysTime.Seconds*1000 + sysTime.SubSeconds;
    
    g_systime_ref = set_time - cur_time;
}

TimerSysTime_t TimerGetSysTime( void )
{
    TimerSysTime_t sysTime = { 0 };
    TimerTime_t curTime = TimerGetCurrentTime();

    sysTime.Seconds = (uint32_t)(curTime/1000);
    sysTime.SubSeconds = (uint16_t)(curTime%1000);

    return sysTime;
}

static void TimeStampsUpdate()
{    
    TimerTime_t old =  RtcGetTimerContext(); 
    TimerTime_t now =  RtcSetTimerContext(); 
    uint32_t DeltaContext = (uint32_t)(now - old);
    
    TimerEvent_t* cur = TimerListHead;
    while(cur) {
        if (cur->Timestamp > DeltaContext)
            cur->Timestamp -= DeltaContext;
        else
            cur->Timestamp = 0 ;
        cur = cur->Next;
    }
}

void TimerInit( TimerEvent_t *obj, void ( *callback )( void *context ) )
{
    obj->Timestamp = 0;
    obj->ReloadValue = 0;
    obj->IsStarted = false;
    obj->IsNext2Expire = false;
    obj->Callback = callback;
    obj->Context = NULL;
    obj->Next = NULL;
}

void TimerSetContext( TimerEvent_t *obj, void* context )
{
    obj->Context = context;
}

void TimerStart( TimerEvent_t *obj )
{
    uint32_t elapsedTime = 0;

    BoardDisableIrq( );

    if( ( obj == NULL ) || ( TimerExists( obj ) == true ) )
    {
        BoardEnableIrq( );
        return;
    }

    obj->Timestamp = obj->ReloadValue;
    obj->IsStarted = true;
    obj->IsNext2Expire = false;

    if( TimerListHead == NULL )
    {
        RtcSetTimerContext( );
        // Inserts a timer at time now + obj->Timestamp
        TimerInsertNewHeadTimer( obj );
    }
    else
    {
        elapsedTime = RtcGetElapsedTime();
        obj->Timestamp += elapsedTime;

        if( obj->Timestamp < TimerListHead->Timestamp )
        {
            TimeStampsUpdate();
            obj->Timestamp -= elapsedTime;
            TimerInsertNewHeadTimer( obj );
        }
        else
        {
            TimerInsertTimer( obj );
        }
    }
    BoardEnableIrq( );
}

static void TimerInsertTimer( TimerEvent_t *obj )
{
    TimerEvent_t* cur = TimerListHead;
    TimerEvent_t* next = TimerListHead->Next;

    while( cur->Next != NULL )
    {
        if( obj->Timestamp > next->Timestamp )
        {
            cur = next;
            next = next->Next;
        }
        else
        {
            cur->Next = obj;
            obj->Next = next;
            return;
        }
    }
    cur->Next = obj;
    obj->Next = NULL;
}

static void TimerInsertNewHeadTimer( TimerEvent_t *obj )
{
    TimerEvent_t* cur = TimerListHead;

    if( cur != NULL )
    {
        cur->IsNext2Expire = false;
    }

    obj->Next = cur;
    TimerListHead = obj;
    TimerSetTimeout( TimerListHead );
}

bool TimerIsStarted( TimerEvent_t *obj )
{
    return obj->IsStarted;
}

void TimerIrqHandler( void )
{
    TimerEvent_t* cur;

    //update timer context for callbacks
    TimeStampsUpdate();

    // Execute immediately the alarm callback
    if ( TimerListHead != NULL )
    {
        cur = TimerListHead;
        TimerListHead = TimerListHead->Next;
        cur->IsStarted = false;
        ExecuteCallBack( cur->Callback, cur->Context );
    }

    // Remove all the expired object from the list
    while( ( TimerListHead != NULL ) && ( ( TimerListHead->Timestamp < RtcGetElapsedTime( ) ) || TimerListHead->Timestamp==0 ) )
    {
        cur = TimerListHead;
        TimerListHead = TimerListHead->Next;
        cur->IsStarted = false;
        ExecuteCallBack( cur->Callback, cur->Context );
    }

    //update timestamps after callbacks
    TimeStampsUpdate();

    // Start the next TimerListHead if it exists AND NOT running
    if( ( TimerListHead != NULL ) && ( TimerListHead->IsNext2Expire == false ) )
    {
        TimerSetTimeout( TimerListHead );
    }
}

void TimerStop( TimerEvent_t *obj )
{
    BoardDisableIrq( );

    TimerEvent_t* prev = TimerListHead;
    TimerEvent_t* cur = TimerListHead;

    // List is empty or the obj to stop does not exist
    if( ( TimerListHead == NULL ) || ( obj == NULL ) )
    {
        BoardEnableIrq( );
        return;
    }

    obj->IsStarted = false;

    if( TimerListHead == obj ) // Stop the Head
    {
        if( TimerListHead->IsNext2Expire == true ) // The head is already running
        {
            TimerListHead->IsNext2Expire = false;
            if( TimerListHead->Next != NULL )
            {
                TimerListHead = TimerListHead->Next;

                //update timestamps after stopping timer
                TimeStampsUpdate();
                TimerSetTimeout( TimerListHead );
            }
            else
            {
                RtcStopTimeout( );
                TimerListHead = NULL;
            }
        }
        else // Stop the head before it is started
        {
            if( TimerListHead->Next != NULL )
            {
                TimerListHead = TimerListHead->Next;
            }
            else
            {
                TimerListHead = NULL;
            }
        }
    }
    else // Stop an object within the list
    {
        while( cur != NULL )
        {
            if( cur == obj )
            {
                if( cur->Next != NULL )
                {
                    cur = cur->Next;
                    prev->Next = cur;
                }
                else
                {
                    cur = NULL;
                    prev->Next = cur;
                }
                break;
            }
            else
            {
                prev = cur;
                cur = cur->Next;
            }
        }
    }
    BoardEnableIrq( );
}

static bool TimerExists( TimerEvent_t *obj )
{
    TimerEvent_t* cur = TimerListHead;

    while( cur != NULL )
    {
        if( cur == obj )
        {
            return true;
        }
        cur = cur->Next;
    }
    return false;
}

void TimerReset( TimerEvent_t *obj )
{
    TimerStop( obj );
    TimerStart( obj );
}

void TimerSetValue( TimerEvent_t *obj, uint32_t value )
{
    TimerStop( obj );
    
    obj->Timestamp = value;
    obj->ReloadValue = value;
}

TimerTime_t TimerGetCurrentTime( void )
{
    return RtcGetTimerValue( ) + g_systime_ref;
}

TimerTime_t TimerGetElapsedTime( TimerTime_t past )
{
    return (TimerGetCurrentTime() - past);
}

static void TimerSetTimeout( TimerEvent_t *obj )
{
    obj->IsNext2Expire = true;

    RtcSetTimeout(obj->Timestamp);
}

TimerTime_t TimerTempCompensation( TimerTime_t period, float temperature )
{
    (void)temperature;
    
    return period;
}

void TimerProcess( void )
{
    //RtcProcess( );
}

void TimerLowPowerHandler( void )
{
    //if( ( TimerListHead != NULL ) && ( TimerListHead->IsRunning == true ) )
    {
        if( HasLoopedThroughMain < 5 )
        {
            HasLoopedThroughMain++;
        }
        else
        {
            HasLoopedThroughMain = 0;
            RtcEnterLowPowerStopMode( );
        }
    }
}
