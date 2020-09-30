#ifndef Response_h
#define Response_h

#include <Arduino.h>
#include <Hmac.h>
#include <Errors.h>

#define RESPONSE_HEADER_SIZE 32
#define RESPONSE_PROTOCOL_ID_OFFSET 0
#define RESPONSE_TIMESTAMP_OFFSET 1
#define RESPONSE_STATUS_CODE_OFFSET 9
#define RESPONSE_CHALLENGE_OFFSET 10

#define RESPONSE_BASE_SIZE RESPONSE_HEADER_SIZE + HMAC_SIZE

// The maximum number of errors to record with a response.
// Additional errors are discarded.
#define ERROR_COUNT_LIMIT 16

/**
 * Base class for all Pipsqueak Protocol network response classes.
 *
 * Because dynamic memory allocation can lead to memory fragmentation
 * over time, instances of this class are designed for re-use. Only
 * one instance of each derived class needs to be allocated for each
 * application thread. Instances are not thread safe except that
 * specific methods are designed to be invoked within ISRs. Each such
 * method is specifically documented as being ISR-safe.
 *
 * Derived classes need to implement getBuffer(), getPayload(),
 * getExpectedSize(), and getExpectedProtocol().
 *
 * Derived classes own the volatile buffer ("buffer") into which response
 * data is written via ISR and the non-volatile buffer ("payload") into
 * which the volatile buffer is copied after the request-response cycle
 * is complete. Only derived classes will know how big these buffers will
 * need to be, and without reliance on dynamic memory allocation, only
 * derived classes can declare them to be of the correct size.
 */
class Response {
  public:
    /**
     * Constructor.
     *
     * Hmac is taken as an argument so that the application need only
     * allocate a single Hmac class instance. This results in minor
     * savings in an application's memory allocation requirements.
     *
     * The optional name is included in debug/log statements.
     */
    Response(Hmac * hmac, const char * name = "Response");

    /**
     * Clears the state of this response.
     */
    void reset();

    /**
     * ISR-invoked method that writes incoming response bytes to the derived
     * classes' buffer. Records the total number of bytes received, but writes
     * no more than getExpectedSize() bytes to the buffer.
     */
    void receiveBytes(void * data, size_t len);

    /**
     * Indicates that either all expected bytes have been received, or that
     * errors have been encountered. Either way, the socket can be closed, and
     * once the socket is closed, ready() can be invoked.
     */
    bool isComplete();

    /**
     * Prepares the completed response for access by application code. Sets the
     * "ready" flag and copies the volatile buffer's contents to the non-volatile
     * buffer.
     *
     * elapsedTime: seconds elapsed from the time when the request was prepared
     *              until the moment this method was invoked.
     */
    void ready(time_t elapsedTime);

    /**
     * Indicates that the request-response cycle is complete and the contents of the
     * response are now ready for access by application code beyond that code
     * responsible for execution of the request-response cycle.
     */
    bool isReady();

    /**
     * Indicates that this instance is being used - or about to be used - in a
     * request-response cycle. Useful as a signal to application code that the
     * instance is not currently ready for re-use. If false, the instance is
     * available for use in support of a new request-response cycle.
     */
    bool isInUse();

    /**
     * Records an error encountered during the request-response cycle.
     * Generally invoked by network client code.
     */
    void addError(ErrorType errorType, int8_t errorCode);

    /**
     * Indicates the presence of absence of errors.
     */
    bool hasErrors();

    /**
     * Indicates the number of errors encountered.
     */
    size_t errorCount();

    /**
     * Returns an error type for an error specified by cardinal position.
     * If the index is out of bounds, ErrorType::None will be returned.
     */
    ErrorType getErrorType(size_t index);

    /**
     * Returns the error code for an error specified by cardinal position.
     * If the index is out of bounds, ERROR_NONE will be returned.
     */
    int8_t getErrorCode(size_t index);

    /**
     * Returns the Unix timestamp returned by the server.
     */
    uint32_t getTimestamp();

    /**
     * Returns the number of seconds that elapsed while the request was
     * being transmitted and the response was being received.
     */
    time_t getElapsedTime();

    /**
     * Sets the challenge value.
     *
     * This will ordinarily be invoked by Request.setChallenge(uint32_t).
     *
     * challenge: an arbitrary number, ideally a new randomly generated number,
     *            matching the value supplied to the Request's setChallenge
     *            method.
     */
    void setChallenge(uint32_t challenge);

    /**
     * Returns the name of this response for logging purposes.
     */
    const char * getName();

  protected:
    /**
     * ISR-invoked method that returns a reference to the volatile buffer
     * allocated by the derived class.
     *
     * This method must be implemented by derived classes, and must include
     * the ICACHE_RAM_ATTR macro. The buffer itself must be declared volatile,
     * not merely cast as a volatile pointer. Implementations should
     * do nothing except returning the pointer.
     */
    virtual volatile byte * getBuffer() = 0;

    /**
     * Rturns a reference to the non-volatile payload buffer allocated by the
     * derived class.
     *
     * This method must be implemented by derived classes. Implementations are not
     * expected to do anything except returning the pointer.
     */
    virtual byte * getPayload() = 0;

    /**
     * ISR-invoked method that returns the number of bytes that are expected to be
     * received.
     *
     * Present implementations assume the value returned will not vary throughout
     * the request-response lifecycle. Bugs are probable if this assumption is
     * violated, meaning that this method should not, for example, rely on values
     * in the response header to determine the overall expected response size.
     *
     * This method must be implemented by derived classes, and must include
     * the ICACHE_RAM_ATTR macro. All methods directly or indirectly invoked by
     * implementations of this method must also include the ICACHE_RAM_ATTR macro,
     * and all variables relied upon, whether directly or indirectly, must be
     * declared volatile.
     */
    virtual size_t getExpectedSize() = 0;

    /**
     * Returns the protocol ID of the request/response pair.
     *
     * This method must be implemented by derived classes. The value returned is
     * likely to be a static value unique to the derived class.
     */
    virtual uint8_t getExpectedProtocol() = 0;

  private:
    Hmac * _hmac;
    const char * _name;
    uint32_t _challenge;
    volatile size_t _bytesReceived;
    size_t _errorCount;
    byte _errors[ERROR_COUNT_LIMIT * 2];
    bool _ready;
    volatile bool _inUse;
    time_t _elapsedTime;

    /**
     * Returns the protocol ID included in the response by the server.
     *
     * Behavior is undefined if the response has errors.
     */
    uint8_t getProtocol();

    /**
     * Returns the status code included in the response by the server.
     *
     * Behavior is undefined if the response has errors.
     */
    uint8_t getStatusCode();

    /**
     * Returns the challenge response value included in the response by the
     * server - a value that is expected to match the challenge value imbedded
     * in the request.
     *
     * Behavior is undefined if the response has errors.
     */
    uint32_t getChallengeResponse();

    /**
     * Returns the offset of the HMAC, which is computed using the return value
     * of getExpectedSize().
     */
    size_t getHmacOffset();

    /**
     * Validates that the protocol matches the expected protocol, recording an error
     * if this proves not to be the case.
     */
    bool inspectProtocol();

    /**
     * Validates that the HMAC included in the response matches the HMAC computed
     * by the device on the response, recording an error if this proves not to be
     * the case.
     */
    bool inspectHmac();

    /**
     * Validates that the challenge value in the response matches the challenge value
     * issued in the request (as determined by the value supplied to setChallenge(uint32_t)),
     * recording an error if this proves not to be the case.
     */
    bool inspectChallenge();

    /**
     * Records errors for each error flag embedded in the status code returned from the
     * server in the response.
     */
    bool inspectStatusCode();
};

#endif // Response_h
