#ifndef Request_h
#define Request_h

#include <Arduino.h>
#include <Hmac.h>
#include "Response.h"

#define REQUEST_HEADER_SIZE 32
#define REQUEST_PROTOCOL_ID_OFFSET 0
#define REQUEST_DEVICE_ID_OFFSET 1
#define REQUEST_TIMESTAMP_OFFSET 5
#define REQUEST_CHALLENGE_OFFSET 10

#define REQUEST_BASE_SIZE REQUEST_HEADER_SIZE + HMAC_SIZE

/**
 * Base class for all Pipsqueak Protocol network request classes.
 *
 * Because dynamic memory allocation can lead to memory fragmentation
 * over time, instances of this class are designed for re-use. While
 * instances of this class are not thread safe, only one should be
 * allocated per thread.
 *
 * The methods of this class are not designed to be invoked within an
 * Interrupt Service Routine (ISR).
 *
 * Derived classes must implement getSize(), getResponse(), and
 * getBuffer().
 *
 * Derived classes that maintain state of their own or that directly
 * modify their buffer must invoke setPopulated() before state updates
 * and should override reset() to reset any internally-maintained
 * state, including modifications to the buffer.
 */
class Request {
  public:
    /**
     * Constructor.
     *
     * Hmac is taken as an argument so that the application need only
     * allocate a single Hmac class instance. This results in minor
     * savings in an application's memory allocation requirements.
     *
     * The optional name is included in debug/log statements.
     *
     * Derived classes must additionally invoke the static initialize
     * method.
     */
    Request(Hmac * hmac, const char * name = "Request");

    /**
     * Indicates whether request content has been populated.
     *
     * Universal header and HMAC values (e.g. device ID, timestamp,
     * challenge) are not considered "request content."
     *
     * Returns false immediately after construction and any call to
     * reset().
     *
     * This signals when a request is ready for transmission,
     * since sending a request without content is pointless.
     *
     * Returns true if this request has been populated with content
     * that is worth transmitting to the server, and false if it
     * is either not yet populated, or has yet to be populated
     * with enough data to make transmission worthwhile.
     */
    bool isPopulated();

    /**
     * Prepares the request and response for transmission.
     *
     * To be invoked immediately prior to each transmission of the request.
     *
     * This method may be invoked more than once without reset() to retry
     * transmission.
     *
     * Each invocation:
     * - Resets, sets the challenge in, and prepares the response
     * - Updates the request timestamp
     * - Re-computes the request HMAC to include the updated timestamp
     * - Resets the request
     * - Sets the inFlight flag
     *
     * The challenge value should be regenerated for each transmission of each
     * request. It is embedded in the request, and coupled with the HMAC, acts
     * as a countermeasure to device attacks involving replay of signed
     * responses. Only if the HMAC is valid and the challenge value returned
     * in the response matches the value sent in the request can the response
     * be considered authentic. That authenticity check is built in to the
     * Response class.
     *
     * now: UNIX timestamp of the time this method is called
     * challenge: an arbitrary number, ideally a new randomly generated number
     *
     * Returns false if the request is not ready to send (e.g. when it is
     * not yet populated) and true otherwise.
     */
    bool ready(time_t now, uint32_t challenge);

    /**
     * Indicates that the request is, is is about to be, transmitted to
     * the server.
     */
    bool isInFlight();

    /**
     * Sets up the request for future re-transmission attempt(s). In essence
     * this is the inverse of ready(). Notably, it preserves the relevant
     * content so that it is not simply lost in the event
     *
     * Each invocation:
     * - Clears the inFlight flag
     * - Clears the timestamp
     * - Clears the challenge
     * - Clears the HMAC.
     * - Resets the response.
     */
    void failed();

    /**
     * Restores the state of this request to its initial state, clearing
     * transient header values, request content, and the HMAC.
     *
     * Derived classes must override this method if they maintain their
     * own internal state or directly (i.e. not via Request methods) modify
     * the contents of their buffer.
     *
     * Overriding methods must:
     * 1. Invoke this method (i.e. Request::reset()) prior to making any
     *    changes that modify the return value of getSize(). That return
     *    value is used by this method to clear the previously-set HMAC.
     * 2. Reset internal  state (counters, sizes, etc) to initial values.
     * 3. Zero out any buffer bytes that were modified by the derived
     *    class subsequent to the latter of construction or the prior
     *    call to reset(). Do not zero out any other bytes, and in
     *    particular, do not zero out the protocolId and deviceId bytes
     *    in the header.
     */
    virtual void reset();

    /**
     * Returns the size of the request in bytes.
     *
     * Derived classes must implement this method.
     *
     * The value returned by this method must never exceed the size of the
     * derived classes' buffer, but may be less than that size.
     *
     * This class does not provide an implementation of this method.
     */
    virtual size_t getSize() = 0;

    /**
     * Returns a pointer to the response class instance that will be
     * populated after sending this request to the server.
     *
     * Derived classes must implement this method.
     *
     * Since the concrete derived Response class is known only to the derived
     * Request class, this class does not provide an implementation of this method.
     */
    virtual Response * getResponse() = 0;

    /**
     * Returns a pointer to the buffer allocated by a derived class to store the
     * request. This buffer's contents will be transmitted to the server when the
     * request is sent.
     *
     * Since dynamic memory allocation can lead to heap fragmentation over time
     * and static allocation is not possible without knowing the amount of space
     * required at compile time, derived classes must allocate their own buffer
     * and return a reference to that buffer in their implementation of this
     * method.
     *
     * This class does not provide an implementation of this method.
     */
    virtual byte * getBuffer() = 0;

    /**
     * The UNIX timestamp provided to the ready(time_t, uint32_t) method, or
     * zero if never invoked or not invoked since latest reset() or failed()
     * call.
     */
    time_t getTimestamp();

    /**
     * Returns the name of this request for logging purposes.
     */
    const char * getName();

  protected:
    /**
     * Performs universal initialization actions. Unsets every byte in the buffer
     * and sets the protocolId and deviceId in the header.
     */
    static void initialize(byte * buffer, size_t bufferSize, uint8_t protocolID, uint32_t deviceID);

    /**
     * Sets the flag indicating that the request is now populated. See isPopulated().
     * Only the reset() method can un-set the isPopulated flag.
     */
    void setPopulated();

  private:
    const char * _name;
    bool _populated;
    bool _inFlight;
    Hmac * _hmac;

    size_t getHmacOffset();
    void setHmac();
    void reset(bool populated);
};

#endif // Request_h
