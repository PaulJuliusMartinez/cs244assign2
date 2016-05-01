#ifndef CONTROLLER_HH
#define CONTROLLER_HH

#include <cstdint>

#define MAX_WINDOW_SIZE 256
#define BETA 0.05
#define RTT_CONGESTION_DETECTION 1.25

/* Congestion controller interface */

class Controller
{
private:
  bool debug_; /* Enables debugging output */

  /* Add member variables here */
  uint64_t datagram_send_times[MAX_WINDOW_SIZE]; /* Keep track of packet departure times */
  double running_rtt;
  float curr_window_size;
  uint64_t caused_md;
  uint64_t reengage_aimd;

public:
  /* Public interface for the congestion controller */
  /* You can change these if you prefer, but will need to change
     the call site as well (in sender.cc) */

  /* Default constructor */
  Controller( const bool debug );

  /* Get current window size, in datagrams */
  unsigned int window_size( void );

  /* A datagram was sent */
  void datagram_was_sent( const uint64_t sequence_number,
			  const uint64_t send_timestamp );

  /* An ack was received */
  void ack_received( const uint64_t sequence_number_acked,
		     const uint64_t send_timestamp_acked,
		     const uint64_t recv_timestamp_acked,
		     const uint64_t timestamp_ack_received );

  /* How long to wait (in milliseconds) if there are no acks
     before sending one more datagram */
  unsigned int timeout_ms( void );
};

#endif
