

string parse_packet(char * packet, size_t len){
  MsgHeader * msg_h = static_cast<MsgHeader*>packet;
  switch (msg_h->packet_type){
    case("L"):
      return parse_login_request(packet, len);
      break;
  }
}

string parse_login_request(char * packet, size_t len){
  LoginRequest * r = static_cast<LoginRequest*>packet;
  char username[] = "S155T1";
  char password[] = "testonly";
  if (strncmp(r->username, username, sizeof(username)) and strncmp(r->password, password, sizeof(password))){
    LoginAccepted la;
    la.length = sizeof(la);
    la.packet_type = 'A';
    memset(la.seq_num, ' ', sizeof(la.seq_num));
    strncpy(la.session, packet.requested_session, sizeof(la.session));
    unsigned int * seq_num = static_cast<int*>(la.seq_num);
    *seq_num = 0;
    return string(static_cast<const char*>(&la), sizeof(la));
  }else{
    LoginRejected lj;
    lj.length = sizeof(lj);
    lj.packet_type = 'J';
    lj.reason = 'A';
    return string(static_cast<const char*>(&lj), sizeof(lj));
  }
}
