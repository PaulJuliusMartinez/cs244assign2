#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

#define T_LOW 80
#define T_HIGH 200
#define DELTA 1
#define ALPHA 0.05

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug ),
  running_rtt ( 0 ),
  curr_window_size ( 10 ),
  caused_md ( 0 ),
  reengage_aimd ( 0 )
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
  uint64_t rtt = timestamp_ack_received -
      datagram_send_times[sequence_number_acked % MAX_WINDOW_SIZE];
  /*

  uint64_t old_rtt = running_rtt;
  running_rtt = BETA * rtt + (1 - BETA) * running_rtt;
  */

  if (rtt > 120 && sequence_number_acked >= reengage_aimd) {
    caused_md = sequence_number_acked;
    // let all outstanding packets clear
    reengage_aimd = caused_md + (uint64_t) curr_window_size;
    curr_window_size /= 2;
  } else {
    curr_window_size += 1 / curr_window_size;
  }

  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock"
         << ", rtt = " << rtt << "ms)"
	 << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{
  return 1000; /* timeout of one second */
}
