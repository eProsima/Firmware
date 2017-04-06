#!/usr/bin/env python

import sys, os
import px_generate_uorb_topic_files

if len(sys.argv) > 1:
    msg_files = sys.argv[1:]
else:
    print("At least one .msg file must be specified")
    exit(-1)

root_path = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
msg_folder = root_path + "/msg"
px_generate_uorb_topic_files.append_to_include_path({msg_folder}, px_generate_uorb_topic_files.INCL_DEFAULT)

out_dir = root_path + "/msgenerated"
print("Files created in: " + out_dir)

uorb_templates_dir = root_path + "/msg/templates/uorb"
urtps_templates_dir = root_path + "/msg/templates/urtps"



uRTPS_TRANS_APP_GEN_TEMPL_FILE = 'general_uRTPS_UART_transmitter.cpp.template'
uRTPS_TRANS_CML_GEN_TEMPL_FILE = 'general_transmitter_CMakeLists.txt.template'
uRTPS_PUBSUBMAIN_GEN_TEMPL_FILE = 'general_uRTPS_UART_PubSubMain.cxx.template'
uRTPS_PUBLISHER_SRC_TEMPL_FILE = 'Publisher.cxx.template'
uRTPS_PUBLISHER_H_TEMPL_FILE = 'Publisher.h.template'
uRTPS_SUBSCRIBER_SRC_TEMPL_FILE = 'Subscriber.cxx.template'
uRTPS_SUBSCRIBER_H_TEMPL_FILE = 'Subscriber.h.template'


for msg_file in msg_files:
	px_generate_uorb_topic_files.generate_idl_file(msg_file, out_dir, urtps_templates_dir, 
		px_generate_uorb_topic_files.INCL_DEFAULT)
	px_generate_uorb_topic_files.generate_topic_file(msg_file, out_dir, urtps_templates_dir, 
		px_generate_uorb_topic_files.INCL_DEFAULT, uRTPS_PUBLISHER_SRC_TEMPL_FILE)
	px_generate_uorb_topic_files.generate_topic_file(msg_file, out_dir, urtps_templates_dir, 
		px_generate_uorb_topic_files.INCL_DEFAULT, uRTPS_PUBLISHER_H_TEMPL_FILE)
	px_generate_uorb_topic_files.generate_topic_file(msg_file, out_dir, urtps_templates_dir, 
		px_generate_uorb_topic_files.INCL_DEFAULT, uRTPS_SUBSCRIBER_SRC_TEMPL_FILE)
	px_generate_uorb_topic_files.generate_topic_file(msg_file, out_dir, urtps_templates_dir, 
		px_generate_uorb_topic_files.INCL_DEFAULT, uRTPS_SUBSCRIBER_H_TEMPL_FILE)

px_generate_uorb_topic_files.generate_uRTPS_general(msg_files, out_dir, uorb_templates_dir, 
				px_generate_uorb_topic_files.INCL_DEFAULT, uRTPS_TRANS_APP_GEN_TEMPL_FILE)

px_generate_uorb_topic_files.generate_uRTPS_general(msg_files, out_dir, uorb_templates_dir, 
				px_generate_uorb_topic_files.INCL_DEFAULT, uRTPS_TRANS_CML_GEN_TEMPL_FILE)

px_generate_uorb_topic_files.generate_uRTPS_general(msg_files, out_dir, urtps_templates_dir, 
				px_generate_uorb_topic_files.INCL_DEFAULT, uRTPS_PUBSUBMAIN_GEN_TEMPL_FILE)


