#include "NetworkHelpers.h"
#include "PDUConstants.h"
#include <iostream>
#include <string>
#include <cstring>

std::string GetField(int start, char * fields, int field_len){
  char field_array[field_len + 1];
  int end = start + field_len;

  std::copy(fields + start, fields + end, field_array);

  field_array[field_len] = '\0';

  std::string field(field_array);

  return field;
}

int StoI(std::string number){
  return std::stoi(number, nullptr, 10);
}

void print_header();
void print_footer();
void Print(void (*f) (char *));
void Register(char * PDU);
void Setup(char * PDU);
void Vision(char * PDU);
void Movement(char * PDU);

void PrintPDU(char * PDU){
  char PDU_type = PDU[PDU_TYPE_INDEX];
  switch(PDU_type){
    case REGISTER :
      Register(PDU);
      break;
    case SETUP    :
      Setup(PDU);
      break;
    case VISION   :
      Vision(PDU);
      break;
    case MOVEMENT :
      Movement(PDU);
      break;
    default  :
      break;
  }
}

void Print(void (*f) (char *)){
  print_header();
  (*f);
  print_footer();
}

void Register(char * PDU){
  std::cout << "REGISTRATION PACKET!" << std::endl;
}

void Setup(char * PDU){
  int len = SETUP_FIELD_LEN;
  std::cout << "SETUP PACKET!" << std::endl;
  std::cout << "ID          : " << GetField(PDU_ID_INDEX, PDU, ID_LEN) << std::endl;
  std::cout << "VISION SIZE : " << GetField(SETUP_VISION_INDEX, PDU, len) << std::endl;
  std::cout << "STEP SIZE   : " << GetField(SETUP_STEP_INDEX, PDU, len)  << std::endl;
  std::cout << "CURRENT ROW : " << GetField(SETUP_CURR_ROW_INDEX, PDU, len)  << std::endl;
  std::cout << "CURRENT COL : " << GetField(SETUP_CURR_COL_INDEX, PDU, len)  << std::endl;
  std::cout << "DEST    ROW : " << GetField(SETUP_DEST_ROW_INDEX, PDU, len)  << std::endl;
  std::cout << "DEST    COL : " << GetField(SETUP_DEST_COL_INDEX, PDU, len)  << std::endl;
}

void Vision(char * PDU){
  int len = VISION_FIELD_LEN;
  std::cout << "VISION PACKET!" << std::endl;
  std::cout << "SEQ NUM     : " << (int)PDU[VISION_SEQ_NUM_INDEX] << std::endl;
  std::cout << "COLLIDED    : " << PDU[VISION_COLLIDED_INDEX] << std::endl;
  std::cout << "ARRIVED     : " << PDU[VISION_ARRIVED_INDEX] << std::endl;
  std::cout << "TIMEOUT     : " << PDU[VISION_TIMEOUT_INDEX] << std::endl;
  std::cout << "CURRENT ROW : " << GetField(VISION_CURR_ROW_INDEX, PDU, len)  << std::endl;
  std::cout << "CURRENT COL : " << GetField(VISION_CURR_COL_INDEX, PDU, len)  << std::endl;
  std::cout << "VISION GRID :" << std::endl;
  for (int i = VISION_GRID_INDEX; i < BUFLEN; i++){
    std::cout << PDU[i];
  }
  std::cout << std::endl;
  std::cout << std::endl;
}

void Movement(char * PDU){
  int len = MOVE_FIELD_LEN;
  std::cout << "MOVEMENT PACKET!" << std::endl;
  std::cout << "ID          : " << GetField(PDU_ID_INDEX, PDU, ID_LEN) << std::endl;
  std::cout << "SEQ NUM     : " << (int)PDU[MOVE_SEQ_INDEX] << std::endl;
  std::cout << "MOVE TO ROW : " << GetField(MOVE_NEXT_ROW_INDEX, PDU, len)  << std::endl;
  std::cout << "MOVE TO COL : " << GetField(MOVE_NEXT_COL_INDEX, PDU, len)  << std::endl;
}

void print_header(){
  std::string header = "==========================";

  std::cout << std::endl;
  std::cout << header << std::endl;
  std::cout << std::endl;

}

void print_footer(){
  std::string footer = "==========================";

  std::cout << std::endl;
  std::cout << footer << std::endl;
  std::cout << std::endl;
}
