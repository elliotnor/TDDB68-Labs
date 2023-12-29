#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct student {
	char *name;
	struct list_elem elem;
};

typedef struct student student;

void insert (struct list *student_list) {
	struct student *newStudent = (struct student*) malloc (sizeof(struct student));
	char *studentName = (char*) malloc (sizeof(char*));
	printf("Input student name: ");
	scanf("%s", studentName);
	newStudent->name = studentName;
	list_push_back(student_list, &(newStudent->elem));
}

void delete (struct list *student_list) {
	char *tempName = (char*) malloc (sizeof(char*));
	printf("Input student name to be removed: ");
	scanf("%s", tempName);
	
	for (struct list_elem *e = list_rbegin (student_list); e != list_rend (student_list);
           	e = list_prev (e))
        {
          	student *s = list_entry(e, student, elem);
			if(!strcmp(s->name, tempName)){
				list_remove(e);
				free(s->name);
				free(s);
				break;
			}
        }
	free(tempName);

}

void list (struct list *student_list) {
	for (struct list_elem *e = list_rbegin (student_list); e != list_rend (student_list);
           e = list_prev (e))
        {
          student *s = list_entry(e, student, elem);
          printf("Student name = %s \n", s->name);
        }
}

void quit (struct list *student_list) {
	while (!list_empty(student_list))
     {
       struct list_elem *e = list_pop_front(student_list);
       student *s = list_entry(e, student, elem);
	   free(s->name);
	   free(s);
     }
	exit(0);
}

int main() {
	struct list student_list;
	list_init (&student_list);
	int opt;

	do {
		printf("Menu:\n");
		printf("1 - Insert student\n");
		printf("2 - Delete student\n");
		printf("3 - List students\n");
		printf("4 - Exit\n");
		scanf("%d", &opt);
		switch (opt) {
			case 1:
				{
					insert(&student_list);
					break;
				}
			case 2:
				{
					delete(&student_list);
					break;
				}
			case 3:
				{
					list(&student_list);
					break;
				}

			case 4:
				{
					quit(&student_list);
					break;
				}
			default:
				{
					printf("Quit? (1/0):\n");
					scanf("%d", &opt);
					if (opt)
						quit(&student_list);
					break;
				}
		}
	} while(1);
		
	return 0;
}
