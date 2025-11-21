/*Alvin Joseph
 */

#include<stdio.h>
#include"minishell.h"

char input_string[30];

int main()
{
	system("clear");
	char prompt[25] = "minishell$: ";
	scan_input(prompt,input_string);
}
