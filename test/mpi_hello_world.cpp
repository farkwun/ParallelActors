// Author: Wes Kendall
// Copyright 2011 www.mpitutorial.com
// This code is provided freely with the tutorials on mpitutorial.com. Feel
// free to modify it for your own use. Any distribution of the code must
// either provide a link to www.mpitutorial.com or keep this header intact.
//
// An intro MPI hello world program that uses MPI_Init, MPI_Comm_size,
//
// MPI_Comm_rank, MPI_Finalize, and MPI_Get_processor_name.
//
#include <mpi.h>
#include <stdio.h>

#include <vector>

int world_size, world_rank;

std::vector< std::vector<char> > my_vecs;
int vision_radius = 1;
int map_cols;
int map_rows;
int seg_rows, seg_cols;
int segment_size;
int top_buf_edge_index = -1;
int bot_buf_edge_index = -1;

int start_index, end_index;
void PrintVector(std::vector<char> vector);
std::vector<char> Unroll2DVector(std::vector< std::vector<char> > segment){
  int i;
  std::vector<char> unrolled_segment;
  std::vector<char> segment_row;

  for (i = 0; i < segment.size(); i++){
    segment_row = segment[i];
    unrolled_segment.insert(std::end(unrolled_segment), 
        std::begin(segment_row), 
        std::end(segment_row));
  }

  return unrolled_segment;
}   

void PrintVector(std::vector<char> vector){
  int i;
  for (i = 0; i < vector.size(); i++){
    printf("%c", vector[i]);
  }
  printf("\n");
}

void PrintSegment(std::vector< std::vector<char> > segment){
  int rows = segment.size();
  int cols = segment[0].size();

  int i,j;

  for (i = 0; i < rows; i++){
    PrintVector(segment[i]);
  }
}

std::vector< std::vector<char> > CreateSegmentFromArray(char * received_array, int array_len){
  int i;
  std::vector<char> new_row;
  std::vector< std::vector<char> > new_segment;
  for(i = 0; i < array_len; i++){
    if((i > 0) && (i % map_cols == 0)){
      new_segment.push_back(new_row);
      new_row.clear();
    }
    new_row.push_back(received_array[i]);
  }
  new_segment.push_back(new_row);
  return new_segment;
}


void UpdateMapFromSegment(int start_index, std::vector< std::vector<char> > segment){
  int i,j;
  printf("Process %d, Updating map segment from index %d, segment size is %d\n", world_rank, start_index, segment.size());
  for (i = start_index; i < start_index + segment.size(); i++){
    //map.UpdateWithRow (int index, std::vector<char> row);
    my_vecs[i] = segment[i-start_index];
  }
}

void SendBottomBuffer(){
  if (world_rank == (world_size - 1)){
    return;
  }
  std::vector< std::vector<char> > bottom_buffer;
  std::vector<char> buffer;
  int i;
  /*for (i = 0; i <= top_buf_edge_index; i++){
    top_buffer.push_back(my_vecs[i]);
    }*/
  for (i = bot_buf_edge_index - vision_radius + 1; i <= bot_buf_edge_index; i++){
    bottom_buffer.push_back(my_vecs[i]);
  }
  buffer = Unroll2DVector(bottom_buffer);
  // printf("My rank is %d and the bottom buffer is:\n", world_rank);
  MPI_Send(&buffer.front(), buffer.size(), 
      MPI_CHAR, world_rank + 1, 0, MPI_COMM_WORLD);
  //  PrintVector(Unroll2DVector(bottom_buffer));
}

void ReceiveBottomBuffer(){
  if (world_rank == (world_size - 1)){
    return;
  }
  char received_vector[vision_radius * seg_cols + 1];
  MPI_Recv(&received_vector, (vision_radius * seg_cols),
      MPI_CHAR, world_rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
  received_vector[vision_radius * seg_cols] = '\0';
  printf("Process %d Received Bottom Vector: %s is size %d\n", world_rank, received_vector, strlen(received_vector));
  UpdateMapFromSegment(bot_buf_edge_index + 1, CreateSegmentFromArray(received_vector, strlen(received_vector)));
}

void SendTopBuffer(){
  if (world_rank == 1){
    return;
  }
  std::vector< std::vector<char> > top_buffer;
  std::vector<char> buffer;
  int i;

  for (i = top_buf_edge_index; i < top_buf_edge_index + vision_radius; i++){
    top_buffer.push_back(my_vecs[i]);
  }
  buffer = Unroll2DVector(top_buffer);
  MPI_Send(&buffer.front(), buffer.size(), 
      MPI_CHAR, world_rank - 1, 0, MPI_COMM_WORLD);
  printf("My rank is %d and I've sent my top buffer\n", world_rank);
}

void ReceiveTopBuffer(){
  if (world_rank == 1){
    return;
  }
  //  std::vector<char> * received_vector = (std::vector<char> *) malloc(vision_radius*seg_cols);
  char received_vector[vision_radius * seg_cols + 1];
  MPI_Recv(&received_vector, (vision_radius * seg_cols),
      MPI_CHAR, world_rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
  received_vector[(vision_radius * seg_cols)] = '\0';
  printf("Received Top Vector: %s is size %d\n", received_vector, strlen(received_vector));
  PrintSegment(CreateSegmentFromArray(received_vector,
        strlen(received_vector)));
  UpdateMapFromSegment(0, CreateSegmentFromArray(received_vector,
        strlen(received_vector)));
  //  PrintVector(received_vector);
}
void PrintMyVecs(){
  int rows = my_vecs.size();
  int cols = my_vecs[0].size();
  int i,j;
  for (i = 0; i < rows; i ++){
    for (j = 0; j < cols; j++){
      printf("%c", my_vecs[i][j]);
    }
    printf("\n");
  }
}

void GetMySegment(int world_rank, int world_size, std::vector< std::vector<char> > full_map){
  map_rows = full_map.size();
  map_cols = full_map[0].size();
  seg_cols = map_cols;
  segment_size = map_rows/(world_size - 1);
  start_index = (world_rank - 1) * segment_size;
  end_index = (world_rank) * segment_size - 1;

  int i,j,k;

  int empty_char = 48;

  std::vector<char> empty_row(map_cols, empty_char);

  if (start_index != 0){
    for (i = 0; i < vision_radius; i++){
      my_vecs.push_back(empty_row);
    }
    top_buf_edge_index = i;
  }

  for (i = start_index; i <= end_index; i++){
    my_vecs.push_back(full_map[i]);
  }

  if (world_rank == world_size - 1){
    for (i = end_index + 1; i < map_rows; i++){
      my_vecs.push_back(full_map[i]);
    }
    end_index = map_rows - 1;
  }else{
    for (i = 0; i < vision_radius; i++){
      my_vecs.push_back(empty_row);
    }
    bot_buf_edge_index = my_vecs.size() - vision_radius - 1;
  }
  seg_rows = my_vecs.size();
  printf("My rank is %d, my start index is %d, my end index is %d, my top_buf_edge is %d, and my bot_buf_edge is %d\n", world_rank, start_index, end_index, top_buf_edge_index, bot_buf_edge_index);
  /* printf("==========================\n");
     PrintMyVecs();
     printf("==========================\n");*/
}

int main(int argc, char** argv) {
  // Initialize the MPI environment. The two arguments to MPI Init are not
  // currently used by MPI implementations, but are there in case future

  int rows = 10;
  int cols = 6;

  int first_char = 65;

  std::vector<char> default_row(cols, first_char);
  std::vector< std::vector<char> > char_vec(rows, default_row);

  int i,j,k;

  for (i = 0; i < rows; i++){
    for (j = 0; j < cols; j++){
      char_vec[i][j] = first_char + k;
      k++;
    }
  }


  //  // implementations might need the arguments.
  MPI_Init(NULL, NULL);

  // Get the number of processes
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);


  // Get the rank of the process
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank == 0){
    /*   for (i = 0; i < rows; i++){
         for(j = 0; j < cols; j++){
         printf("%c", char_vec[i][j]);
         }
         printf("\n");
         }*/
  }else{
    GetMySegment(world_rank, world_size, char_vec);
    SendBottomBuffer();
    ReceiveTopBuffer();
    SendTopBuffer();
    ReceiveBottomBuffer();
    printf("==========================\n");
    PrintMyVecs();
    printf("==========================\n");
  }

  /*// Print off a hello world message
    printf("Hello world from processor %s, rank %d out of %d processors\n",
    processor_name, world_rank, world_size);
    */
  // Finalize the MPI environment. No more MPI calls can be made after this
  MPI_Finalize();
}
