//	g++ softwarelist_Linux.cpp -o softwarelist_Linux
//	./softwarelist_Linux
//	vim /tmp/softwares.list
#ifndef MACOSX

#include <stdio.h>
#include <dirent.h> 
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <iostream>

using namespace std;

#define BUFFER_SIZE      1024
#define APPLICATIONS_DIR "/usr/share/applications"
#define SORTED_FILE		 "/tmp/softwares.list.tmp"
#define UNSORTED_FILE	 "/tmp/softwares.list.tmp_1"
#define SOFTWARELIST_FILE                       "/tmp/softwares.list"
#define LARGE_BUFFER                                    3000

struct node
{
	char *name;
	char status;
	char date[30];
	struct node *next;
};

struct node*    getPackageStatus();
void                            deletePackageList();
int                             addToNotificationList(char *software, char status, char *date);
int                             isNotifiedBefore(char *software);
int                             findUninstalledSoftwares();
int                             findInstalledSoftwares();



char* MW_strltrim(char *str)
{
	unsigned int p = 0;
	if(str != NULL)
	{
		while(isspace(str[p++]) && p < strlen(str));
		{       if(str[p] == '\0' && isspace(str[p - 1]))
			str[0] = 0;
			else if(--p > 0)
			{
				printf("str='%s'\t p='%d'\tstr+p='%s'\t strlen(str)='%d'\n",str,p, str + p, strlen(str));
				printf("strlen(str)='%d'\tstrlen(str+p)='%d'\n",strlen(str),strlen(str+p));
				//                      strncpy(str, str + p, strlen(str));
				strcpy(str, str + p);
				printf("str='%s'\n",(str));
			}
		}
	}

	return str;
}
char* MW_strrtrim(char *str)
{
	int p;
	if(str != NULL)
	{
		p = strlen(str) - 1;
		while(isspace(str[p]) && --p >= 0);
		str[++p] = 0;
	}
	return str;
}

char* MW_strtrim(char *str)
{
	if(str != NULL)
	{
		str = MW_strltrim(str);
		str = MW_strrtrim(str);
	}

	return str;
}
char* strrncmp(char *str1, char *str2)
{
	int len1, len2;
	if(str1 == NULL || str2 == NULL)
	{
		printf( "Can not compare a null string\n");
		return NULL;
	}
	len1 = strlen(str1);
	len2 = strlen(str2);
	while(len1-- > 0 && len2-- > 0)
	{
		if(str1[len1] != str2[len2])
		{
			return NULL;
		}
	}
	return(str1 + (strlen(str1) - strlen(str2)));
}


static struct node *softwareList = NULL;

///Returns installed & uninstalled package list.
/**
 * \return struct node* The head to the linked list
 * \return NULL On failure
 * \sa deletePackageList()
 * \sa findInstalledSoftwares()
 * \sa findUninstalledSoftwares()
 */
struct node* getPackageStatus()
{
	/**
	 * Old list of softwares will be deleted if exists before creating new list in memory.
	 */
	deletePackageList();
	/**
	 * Newly installed softwares will be added into the list.
	 */
	if(findInstalledSoftwares() != 0)
	{
		while(softwareList != NULL)
		deletePackageList();
		return NULL;
	}
	/**
	 * The softwares which were installed, but now uninstalled from host will be added into the list.
	 */
	if(findUninstalledSoftwares() != 0)
	{
		while(softwareList != NULL)
		deletePackageList();
		return NULL;
	}
	/**
	 * The list of currently installed softwares will be stored on disk for future checking.
	 */
	if(rename(SORTED_FILE, SOFTWARELIST_FILE) != 0)
	{
		while(softwareList != NULL)
		deletePackageList();
		return NULL;
	}
	/**
	 * In case of finding installed or uninstall softwares, or creating the file on disk, it will return NULL.
	 */
		while(softwareList != NULL)
	return softwareList;
}

///Deletes all existing nodes in software list.
void deletePackageList()
{
	struct node *temp;
	/**
	 * It deletes all nodes and members which are allocated dynamic memory while creating the node.
	 */
	while(softwareList != NULL)
	{
		temp = softwareList;
		softwareList = softwareList->next;
		free(temp->name);
		free(temp);
	}
	return;
}

///Adds newly installed software details to the list.
/**
 * \return -1 On error
 * \return 0 On success
 */
int findInstalledSoftwares()
{
	DIR *directory;
	struct dirent *ent;
	struct stat stats;
	struct tm *timeinfo;
	char buffer[BUFFER_SIZE], filePath[BUFFER_SIZE], date[30], *cPtr = NULL;
	FILE *inputFile, *outputFile;

	/**
	 * All software details will be stored in a temporary file which is not sorted.
	 */
	outputFile = fopen(UNSORTED_FILE, "w+");
	if(outputFile == NULL)
	{
		return -1;
	}
	/*File names ends with '.desktop' in '/usr/share/applications' directory will be used to get the software details.*/
	directory = opendir(APPLICATIONS_DIR);
	if(directory == NULL)
	{
		fclose(outputFile);
		return -1;
	}
	while ((ent = readdir (directory)) != NULL)
	{
		cPtr = strrncmp(ent->d_name, ".desktop");
		/**
		 * Files which does not ends with '.desktop' in their names, will be ignored.
		 */
		if(cPtr == NULL)
		{
			continue;
		}
		sprintf(filePath, "%s/%s", APPLICATIONS_DIR, ent->d_name);
		/**
		 * If file's statastics could not obtained, or open for reading, those file will be ignored.
		 */
		if(stat(filePath, &stats) != 0)
		{
			continue;
		}
		inputFile = fopen(filePath, "r");
		if(inputFile == NULL)
		{
			continue;
		}
		while(fgets(buffer, BUFFER_SIZE, inputFile) != NULL)
		{
			/**
			 * One line at a time will be read from file, the newline character in line will be removed.
			 */
			cPtr = strstr(buffer, "\n");
			if(cPtr != NULL)
			{
				*cPtr = '\0';
			}
			MW_strtrim(buffer);
			/**
			 * Lines that doesn't starts with 'Name=' will be ignored.
			 */
			if(strncmp(buffer, "Name=", 5) != 0)
			{
				continue;
			}
			cPtr = strstr(buffer, "=");
			if(cPtr == NULL)
			{
				continue;
			}
			cPtr++;
			/**
			 * The value field of 'Name=' will be strored in temporary file.
			 */
			if(fputs(cPtr, outputFile) < 0)
			{}
			if(fputs("\n", outputFile) < 0)
			{}
			/**
			 * We keep track of which software names has been sent to the escan server. If software name is already sent to the server, it will not be added into the software list in memory to be sent to the server.
			 */
			if(isNotifiedBefore(cPtr) == 1)
			{
				break;
			}
			/**
			 * Software details those are not yet sent to the server will be added into the list. The installation date will be taken as (.desktop) file creation date. The date format would be '[DD/MM/YYYY] [DD/MMM/YYYY]'. Failed to add any of the software into the list, will be treated as an error and -1 will be returned.
			 */
			timeinfo = localtime(&(stats.st_ctime));
			strftime(date, sizeof(date), "[%d/%b/%Y] [%d/%m/%Y]", timeinfo);
			if(addToNotificationList(cPtr, 0, date) != 0)
			{
				fclose(inputFile);
				closedir(directory);
				return -1;
			}
			break;
		}
		fclose(inputFile);
	}
	closedir(directory);
	fclose(outputFile);
	/**
	 * The temporary file will be sorted and renamed. Failure case of sorting and renaming the file on disk will be ignored.
	 */
	if(access(UNSORTED_FILE, F_OK) == 0)
	{
		sprintf(buffer, "sort -u -o %s %s && rm -f %s", SORTED_FILE, UNSORTED_FILE, UNSORTED_FILE);
		if(system(buffer) != 0)
		{}
	}
	return 0;
}

//************************************************************************************

//This function check that whether given software is notified before or not.
//It returns 1 for notified software, 0 otherwise

//************************************************************************************

///Detects software which details is not sent to the server before.
/**
 * \param software Name of the software
 * \return 0 When software details need to be sent to the server
 * \return 1 When software details had sent before to the server
 */
int isNotifiedBefore(char *software)
{
	FILE *historyFile;
	char buffer[BUFFER_SIZE], *cPtr;
	static int c=0;

	/**
	 * It checks given software name in softwares.list file. If file is not exist then it will be created in whic case all softwares are unknown to the server.
	 */
	historyFile = fopen(SOFTWARELIST_FILE, "r");
	if(historyFile == NULL)
	{
		//It might happened for first time, let's create new one 
		historyFile = fopen(SOFTWARELIST_FILE, "w+");
		if(historyFile == NULL)
		{
			return -1;
		}
	}
	while(fgets(buffer, BUFFER_SIZE, historyFile) != NULL)
	{
		/**
		 * Each line in file will be read one at a time and newline character will be removed from that line before comparison. If any entry in file matches, it stop comparing and returns, otherwise it compares till last line in the file.
		 */
		cPtr = strstr(buffer, "\n");
		if(cPtr != NULL)
		{
			*cPtr = '\0';
		}
		if(strcmp(buffer, software) == 0)
		{
			cout<<"buffer ='"<<buffer<<"'\t"<<"software='"<<software<<"\n";
			cout<<"count ="<<++c<<endl;
			fclose(historyFile);
			return 1;
		}
	}
	fclose(historyFile);
	return 0;
}

///Creates and adds a node to the software linked list.
/**
 * \param software Name of the software
 * \param status 0 for installed, 1 for uninstalled
 * \param date Installation date
 * \return 0 On success
 * \return -1 On error
 */
int addToNotificationList(char *software, char status, char *date)
{
	if(software == NULL)
	{
		return -1;
	}
	/**
	 * A node will be created by allocating required memory and then added at beginning of the list.
	 */
	struct node *temp = (struct node *) malloc(sizeof(struct node));
	if(temp == NULL)
	{
		return -1;
	}
	temp->name = (char *) malloc( sizeof(char) * ( strlen(software) + 1 ) );
	if(temp->name == NULL)
	{
		free(temp);
		return -1;
	}
	strcpy(temp->name, software);
	strcpy(temp->date, date);
	temp->status = status;
	temp->next = softwareList;
	softwareList = temp;
	return 0;
}

///Adds uninstalled software details to the software list.
/**
 * \return 0 On success
 * \return -1 On error
 */
int findUninstalledSoftwares()
{
	FILE *historyFile, *outputFile;
	int isEntryFound;
	char buffer[BUFFER_SIZE], oldPackageName[BUFFER_SIZE], newPackageName[BUFFER_SIZE], *cPtr = NULL;

	/**
	 * The software.list, and sorted file of newly installed softwares software.list.tmp must be exist.
	 */
	historyFile = fopen(SOFTWARELIST_FILE, "r");
	if(historyFile == NULL)
	{
		return -1;
	}
	outputFile = fopen(SORTED_FILE, "r");
	if(outputFile == NULL)
	{
		fclose(historyFile);
		return -1;
	}
	/**
	 * Each name from software.list file will be compared with the all names in the software.list.tmp file. Both files are sorted in this case so that it takes less coparisons when package name is exist in both files.
	 */
	while(fgets(oldPackageName, BUFFER_SIZE, historyFile) != NULL)
	{
		isEntryFound = 0;
		/**
		 * To compare name in software.list with all names in software.list.tmp, pointer will be set to the beginning.
		 */
		if(fseek(outputFile, 0, SEEK_SET) != 0)
		{
			fclose(outputFile);
			fclose(historyFile);
			return -1;
		}
		/**
		 * The newline character in software.list and software.list.tmp file will be removed from lines before comparison.
		 */
		cPtr = strstr(oldPackageName, "\n");
		if(cPtr != NULL)
		{
			*cPtr = '\0';
		}
		while(fgets(newPackageName, BUFFER_SIZE, outputFile) != NULL)
		{
			cPtr = strstr(newPackageName, "\n");
			if(cPtr != NULL)
			{
				*cPtr = '\0';
			}
			if(strcmp(oldPackageName, newPackageName) == 0)
			{
				/**
				 * If any name matches, comparison will be stopped and software name will not be added in list. If name is not found in new list, then only name will be added in linked list.
				 */
				isEntryFound = 1;
				break;
			}
		}
		if(isEntryFound == 0)
		{
			/**
			 * The date information for uninstalled software would be '[00/000/0000] [00/00/0000]'.
			 */
			strcpy(buffer, "[00/000/0000] [00/00/0000]");
			if(addToNotificationList(oldPackageName, 1, buffer) != 0)
			{
				//fclose(historyFile);
				//return -1;
			}
		}
	}
	fclose(historyFile);
	fclose(outputFile);
	return 0;
}

int main()
{
	getPackageStatus( );

}
#endif
