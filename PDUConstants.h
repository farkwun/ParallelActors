//===================================
// include guard
#ifndef PDUCONSTANTS_H
#define PDUCONSTANTS_H

const static int BUFLEN          = 1024;
const static int ID_LEN          = 10;
const static int DATALEN         = 1013;
const static int SETUP_FIELD_LEN = 10;


const static char REGISTER    = 'R';
const static char SETUP       = 'S';
const static char ACKNOWLEDGE = 'A';
const static char ERROR       = 'E';

const static int PDU_TYPE_INDEX = 0;
const static int PDU_ID_INDEX   = 1;
const static int PDU_DATA_INDEX = 11;

const static int ACK_TYPE_INDEX = 1;

const static int SETUP_VISION_INDEX   = 11;
const static int SETUP_STEP_INDEX     = 21;
const static int SETUP_CURR_ROW_INDEX = 31;
const static int SETUP_CURR_COL_INDEX = 41;
const static int SETUP_DEST_ROW_INDEX = 51;
const static int SETUP_DEST_COL_INDEX = 61;

#endif /* PDUCONSTANTS_H */
