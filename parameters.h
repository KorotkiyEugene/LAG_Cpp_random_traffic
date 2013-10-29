/* -----------------------------------------------------------------------------
 * (C)2012 Korotkyi Ievgen
 * National Technical University of Ukraine "Kiev Polytechnic Institute"
 * -----------------------------------------------------------------------------
 */

#ifndef PARAMETERS_H
#define PARAMETERS_H

const bool trace = true;
const bool debug = false;

const int router_pl_num = 2;
const int router_num_pls_on_entry = 1;
const int router_num_pls_on_exit = 2;
const int router_buf_len = 4;
const int router_trunk_num = 5;

const int network_y = 8;
const int network_x = 8;

const double sim_injection_rate = 0.45;
const int sim_packet_length = 5;
const int sim_warmup_packets = 100;
const int sim_measurement_packets = 1000;

const int channel_latency = 0;
const int channel_data_width = 16; //shall not exceed 32 bits

#endif // PARAMETERS_H
