/*

			THE OPCODE FIELD IN "Instr.h" REPRESENTS THE FOLLOWING NUMBERS :

						LOAD = 0
						LOADI = 1
						STORE = 2
						ADD = 3
						SUB = 4
						MUL = 5
						AND = 6
						XOR = 7
						READ = 8
						WRITE = 9

*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "InstrUtils.h"
#include "Utils.h"

void printsaved(int saved[], int x);
int searchsaved(int saved[], int x, int item);

// ---------------------------------------------------------------- | PRINT SAVED VARS

void printsaved(int saved[], int x){

			int i = 0;
			printf("\nSaved Variables : \n");

			for(i = 0; i < x; i++){
					printf("%d ", saved[i]);
			}
			printf("\n");

}

// ---------------------------------------------------------------- | SEARCH SAVED VARS

int searchsaved(int saved[], int x, int item){

			int i = 0;
			for(i = 0; i < x; i++){
					if(saved[i] == item){
						return 1;
					}
			}
			return 0;

}

// ---------------------------------------------------------------- | SEARCH SAVED VARS

void deletenode(Instruction *head, Instruction *deleting){

			 if(head == NULL || deleting == NULL){
			 			return;
			 }

			 if(head == deleting){
					 head = deleting->next;
			 }

			 if(deleting->next != NULL)
					 deleting->next->prev = deleting->prev;

			 if(deleting->prev != NULL){
			 			deleting->prev->next = deleting->next;
			 }

			 free(deleting);
			 return;

}

// ---------------------------------------------------------------- | MAIN()

int main(){

			Instruction *head, *tail, *current;

			int saved[1000]; // Saved Variables As Ints
			int x = 0; // Position Var For 'saved' Array

			int i = 0; // Used For For Loops

			head = ReadInstructionList(stdin);
			current = head;

			if(!head) { // Head Node Of Instructions Is NULL
						WARNING("No instructions\n");
						exit(EXIT_FAILURE);
			}

			else{
						// Get Tail
						while(current->next != NULL){
									current = current->next;
									tail = current;
						}
						current = tail;
						//printf("TAIL : %d\n", current->opcode);
			}

			// ---------------------------------------------------------------------------| Set Criticals Known By Default

						while(current != NULL){

									// Write
									if(current->opcode == 9){
												current->critical = 'Y';
															if(searchsaved(saved, x, current->field1) == 0){ // Not In Saved Array
																		saved[x] = current->field1;
																		x++;
															}
									}

									// Read
									if(current->opcode == 8){
												current->critical = 'Y';
									}

									// Move To Next Instructions
									current = current->prev;

						}

			// ---------------------------------------------------------------------------| Backtrack Variables

						current = tail;
						while(current != NULL){

									//Loop Through Each Saved Variable
									for(i = 0; i < x; i++){

												// -----------------------------| Every Other Opcode

												if(current->opcode != 1 && current->opcode != 0){
															if(current->field1 == saved[i] || current->field2 == saved[i] || current->field3 == saved[i]){
																		current->critical = 'Y';
																					if(searchsaved(saved, x, current->field1) == 0 && current->field1 != 0){
																								saved[x] = current->field1;
																								x++;
																					}
																					if(searchsaved(saved, x, current->field2) == 0 && current->field2 != 0){
																								saved[x] = current->field2;
																								x++;
																					}
																					if(searchsaved(saved, x, current->field3) == 0 && current->field3 != 0){
																								saved[x] = current->field3;
																								x++;
																					}
															}
												}

												// -----------------------------| LOADI

												else if(current->opcode == 1){
															if(current->field1 == saved[i]){
																		current->critical = 'Y';
																					if(searchsaved(saved, x, current->field1) == 0 && current->field1 != 0){
																								saved[x] = current->field1;
																								x++;
																					}
															}
												}

												// -----------------------------| LOAD

												else if(current->opcode == 0){
															if(current->field1 == saved[i]){
																		current->critical = 'Y';
																					if(searchsaved(saved, x, current->field1) == 0 && current->field1 != 0){
																								saved[x] = current->field1;
																								x++;
																					}
																					if(searchsaved(saved, x, current->field2) == 0 && current->field2 != 0){
																								saved[x] = current->field2;
																								x++;
																					}
															}
												}
									}
									// Move To Next Instructions
									current = current->prev;
						}

			// ---------------------------------------------------------------------------| Reroute Linked List And Free() Non Critical Instructions

						current = head;
						while(current != NULL){

									if(current->critical == 'Y'){
												//printf("Saved %d\n", current->opcode);
									}

									else{
												deletenode(head, current);
									}

									// Move To Next Instructions
									current = current->next;
						}

			// ---------------------------------------------------------------------------|

			if(head){
						//printf("\nInstructions After Reroute :\n");
						PrintInstructionList(stdout, head);
						DestroyInstructionList(head);
			}
			return EXIT_SUCCESS;
}
