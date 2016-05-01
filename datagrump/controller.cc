#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

#define T_LOW 60
#define T_HIGH 150
#define DELTA 2
#define ALPHA 0.05
#define BETA 0.2

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug ),
  running_rtt ( 0 ),
  curr_window_size ( 60 ),
  rtt_diff ( 0 ),
  min_rtt ( 5000 ),
  prev_rtt ( 100 ),
  consecutive_decreases ( 0 )
{
}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  unsigned int the_window_size = (unsigned int) curr_window_size;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << the_window_size << endl;
  }

  return the_window_size;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp )
                                    /* in milliseconds */
{
  datagram_send_times[sequence_number % MAX_WINDOW_SIZE] = send_timestamp;

  if ( false ) { //debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << endl;
  }
}

/* An ack was received */
void Controller::ack_received( const uint64_t sequence_number_acked,
			       /* what sequence number was acknowledged */
			       const uint64_t send_timestamp_acked,
			       /* when the acknowledged datagram was sent (sender's clock) */
			       const uint64_t recv_timestamp_acked,
			       /* when the acknowledged datagram was received (receiver's clock)*/
			       const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{
  int64_t rtt = timestamp_ack_received -
      datagram_send_times[sequence_number_acked % MAX_WINDOW_SIZE];

  int64_t new_rtt_diff = rtt - prev_rtt;
  prev_rtt = rtt;
  rtt_diff = (1 - ALPHA) * rtt_diff + ALPHA * new_rtt_diff;

  if (rtt <= min_rtt) {
    min_rtt = rtt;
  }

  // Get some data first
  if (sequence_number_acked < 20) return;

  double normalized_gradient = rtt_diff / min_rtt;
  if (normalized_gradient < 0) {
    consecutive_decreases += 1;
  } else {
    consecutive_decreases = 0;
  }

  if (rtt < T_LOW) {
    cerr << "rtt < T_LOW" << endl;
    curr_window_size += DELTA / curr_window_size;
  } else if (rtt > T_HIGH) {
    cerr << "rtt > T_HIGH" << endl;
    curr_window_size = curr_window_size * (1 - BETA * (1 - T_HIGH / rtt));
  } else if (normalized_gradient <= -0.001) {
    cerr << "normalized_gradient <= 0" << endl;
    curr_window_size += DELTA * 1 / curr_window_size;
  } else {
    cerr << "normalized_gradient > 0" << endl;
    curr_window_size = curr_window_size * (1 - BETA * normalized_gradient);
  }

  if (curr_window_size < 1) {
    curr_window_size = 1;
  }

  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock"
         << ", rtt = " << rtt << "ms)"
         << ", rtt_diff = " << rtt_diff
         << ", normalized_gradient = " << normalized_gradient
	 << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{
  return 1000; /* timeout of one second */
}
