static int state = WAIT;
  switch(state){
    case WAIT:
      if(!RX && idle){
        hard_sync = 1;
        state = HARDSYNC;
      } else if (!RX && !idle){
        soft_sync = 1;
        state = SOFTSYNC;
      }
      break;
    case HARDSYNC:
      hard_sync = 0;
      if(RX)
        state = WAIT;
      break;
    case SOFTSYNC:
      soft_sync = 0;
      if(RX)
        state = WAIT;
      break;
  }