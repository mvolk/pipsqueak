#ifndef PipsqueakState_h
#define PipsqueakState_h

#include <Arduino.h>
#include <Errors.h>
#include <Response.h>
#include <PipsqueakConfig.h>
#include <TelemetryProtocol.h>

// Un-comment to enable detailed debug statements to Serial
// #define DEBUG_PIPSQUEAK_STATE

#define STATUS_EVENT_QUEUE_DEPTH_LIMIT 1024
#define STATUS_EVENT_QUEUE_SIZE STATUS_EVENT_SIZE * STATUS_EVENT_QUEUE_DEPTH_LIMIT
#define REQUEST_SUCCESS_QUEUE_SIZE 4

#define INITIALIZATION_WINDOW_MILLIS 15000

/**
 * Provides and maintains shared state.
 * Records status events.
 *
 * Neither thread-safe nor ISR-safe.
 */
class PipsqueakState {
  public:
    PipsqueakState();

    /** Invoke once before use. */
    void setup();

    /** Invoke on every main loop iteration. */
    void loop();

    /**
     * Returns the persisted configuration for
     * this device.
     */
    PipsqueakConfig * getConfig();

    /**
     * Indicates whether system initialization is complete.
     */
    bool isInitialized();

    /**
     * Indicates whether it is "safe" (subject to the
     * disclaimers) to operate the heater or chiller.
     */
    bool isSafeToOperate();

    /**
     * Indicates whether requests are generally succeeding.
     */
    bool isServerConnectionHealthy();

    /**
     * Indicates whether the device's clock is sufficiently
     * synchronized with the server's clock.
     *
     * Precise synchronization isn't necessary, but the two
     * clocks need to be within a second or two of each other.
     */
    bool isClockSynchronized();

    /**
     * Updates the clock sync state.
     */
    void setClockSynchronized(bool synchronized);

    /**
     * Indicates whether the onboard temperature sensor has
     * been detected.
     */
    bool isBoardSensorDetected();

    /**
     * Updates the board sensor detection state.
     */
    void setBoardSensorDetected(bool sensorDetected);

    /**
     * Returns the board temperature in degrees Celsius.
     */
    float getBoardTemperature();

    /**
     * Updates the board temperature in degrees Celsius.
     */
    void setBoardTemperature(float temperature);

    /**
     * Indicates whether the remote temperature sensor has
     * been detected.
     */
    bool isRemoteSensorDetected();

    /**
     * Updates the remote sensor detected state.
     */
    void setRemoteSensorDetected(bool sensorDetected);

    /**
     * Returns the remote temperature in degrees Celsius.
     */
    float getRemoteTemperature();

    /**
     * Updates the remote temperature in degrees Celsius.
     *
     * Produces a temperature observation status event if
     * the clock is synced and the temperature has changed.
     *
     * If the clock is not synced, the then-current temperature
     * will be recorded in a status event upon next clock sync.
     *
     * Produces an error status event if the temperature
     * is NAN and either this is the first observation or
     * the previous observation was not NAN.
     */
    void setRemoteTemperature(float temperature);

    /**
     * Updates the remote temperature setpoint in degrees
     * Celsius.
     *
     * Delegates to PipsqueakConfig and additionally generates
     * a setpoint status event provided the clock is synced.
     *
     * If the clock is not synced, the then-current setpoint
     * will be reported in a status event upon next clock
     * sync.
     */
    void setRemoteTemperatureSetpoint(float setpoint);

    /**
     * Generates an error status event.
     */
    void recordError(ErrorType errorType, int8_t errorCode);

    /**
     * Generates an error status event for each error associated
     * with the provided response.
     */
    void recordErrors(Response * response);

    /**
     * Generates a heater pulse status event.
     */
    void recordHeaterPulse(uint32_t pulseDuration, uint8_t percentPower, uint32_t recoveryDuration);

    /**
     * Generates a chiller pulse status event.
     */
    void recordChillerPulse(uint32_t pulseDuration, uint32_t recoveryDuration);

    /**
     * Indicates whether there are status events in the queue.
     */
    bool hasStatusEvents();

    /**
     * Removes and returns the oldest status event from the queue.
     *
     * The returned status event's state may be mutated upon the
     * next call to any method of this class. Call the event's
     * write(byte *) method to preserve its state if that state
     * needs to be accessed later.
     */
    StatusEvent * dequeueStatusEvent();

  private:
    PipsqueakConfig _config;
    StatusEvent _statusEvent;
    bool _systemInitialized;
    bool _wifiInitialized;
    bool _clockInitialized;
    bool _clockSynchronized;
    bool _boardSensorInitialized;
    bool _boardSensorDetected;
    bool _boardTemperatureInitialized;
    float _boardTemperature;
    bool _overheated;
    bool _remoteSensorInitialized;
    bool _remoteSensorDetected;
    bool _remoteTemperatureInitialized;
    float _remoteTemperature;
    byte _statusEventQueue[STATUS_EVENT_QUEUE_SIZE];
    size_t _statusEventQueueCursor;
    size_t _statusEventQueueDepth;
    bool _requestSuccess[REQUEST_SUCCESS_QUEUE_SIZE];
    size_t _requestSuccessCursor;

    void enqueueStatusEvent();
    void advanceStatusEventQueueCursor();
};

#endif // PipsqueakState.h
