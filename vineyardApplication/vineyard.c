#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <signal.h>

#define _CRT_SECURE_NO_WARNINGS


void Menu();
void New_Applicant();
void Modify_Applicant();
void Delete_Applicant();
void Display_List();
void Display_By_Day();

#define MAX_DAYS 5

typedef struct Applicants {
    char name[30];
    char days[MAX_DAYS][10];
    int num_days;
    int id;
} applicant;


const char *day;

#define MAX_WORKERS_PER_BUS 5
#define MAX_BUSES 2

// Define a structure for the message
typedef struct {
    long mtype;
    int workers_brought;
} summary_message;

void display_bus_ready_signal(int signum) {
    printf("Bus is ready to load workers.\n");
}

void send_summary_to_vineyard(int workers_brought) {
    key_t key;
    int msgid;

    // Generate a unique key for the message queue
    key = ftok("main", 65);

    // Create a message queue and get its ID
    msgid = msgget(key, 0666 | IPC_CREAT);

    // Prepare the message
    summary_message msg;
    msg.mtype = 1;
    msg.workers_brought = workers_brought;

    // Send the message
    msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0);

    printf("Sent summary message with %d workers brought\n", msg.workers_brought);
}

int count_workers_for_day(const char *day) {
    applicant a;
    FILE *f;
    int count = 0;
    int i;

    f = fopen("applicants.txt", "r");
    if (f == NULL) {
        perror("Error opening file");
        return 0;
    }

    while (fread(&a, sizeof(applicant), 1, f) == 1) {
        bool found = false;
        for (i = 0; i < a.num_days; i++) {
            if (strcmp(a.days[i], day) == 0) {
                found = true;
                count++;
                break;
            }
        }
        if (found) {
            printf("Read worker: %s\n", a.name);
        }
    }

    fclose(f);
    return count;
}

void Display_By_Day_Pipe(int write_fd, const char *day, int offset, int max_workers) {
    applicant a;
    FILE *f;
    int worker_count = 0;
    int total_workers_sent = 0;
    f = fopen("applicants.txt", "r");

    while (fread(&a, sizeof(applicant), 1, f) && total_workers_sent < max_workers) {
        for (int i = 0; i < a.num_days; i++) {
            if (strcmp(a.days[i], day) == 0) {
                if (worker_count >= offset) {
                    write(write_fd, &a, sizeof(applicant));
                    total_workers_sent++;
                    if (total_workers_sent >= max_workers) {
                        break;
                    }
                }
                worker_count++;
                break;
            }
        }
    }
    fclose(f);
}

void Start_Worker_Shuttle(int i, int worker_per_bus, const char *day, int offset)
{
    int pipe_fd[2];
    pid_t pid;

    if (pipe(pipe_fd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();

    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        // Child process
        // Read the worker list from the parent process
        close(pipe_fd[1]); // Close the write end of the pipe in the child process
        applicant workers[MAX_WORKERS_PER_BUS];
        int i = 0;
        while (read(pipe_fd[0], &workers[i], sizeof(applicant)) > 0)
        {
            printf("Worker: %s\n", workers[i].name);
            i++;
        }
        close(pipe_fd[0]);

        // Send summary message to vineyard via message queue
        send_summary_to_vineyard(i);

        exit(EXIT_SUCCESS);
    }
    else
    {
        // Parent process
        // Send the list of workers to the child process
        close(pipe_fd[0]); // Close the read end of the pipe in the parent process
        Display_By_Day_Pipe(pipe_fd[1], day, offset, worker_per_bus);
        close(pipe_fd[1]);

        // Wait for the child process to terminate
        wait(NULL);
    }
}


void Start_Workers(const char *day) {
    int worker_count = count_workers_for_day(day);
    printf("Total workers found for day: %d\n", worker_count);

    if (worker_count == 0) {
        printf("No workers available for the day.\n");
        return;
    }

    int buses_needed = (worker_count + MAX_WORKERS_PER_BUS - 1) / MAX_WORKERS_PER_BUS;
    buses_needed = buses_needed > MAX_BUSES ? MAX_BUSES : buses_needed;

    pid_t pid = fork();

    if (pid == 0) { // Child process
        int offset = 0;
        for (int i = 0; i < buses_needed; ++i) {
            int worker_per_bus = worker_count > MAX_WORKERS_PER_BUS ? MAX_WORKERS_PER_BUS : worker_count;
            worker_count -= worker_per_bus;

            printf("Bus %d is carrying %d workers.\n", i + 1, worker_per_bus);
            if (worker_per_bus == MAX_WORKERS_PER_BUS) {
                printf("Bus %d is full.\n", i + 1);
            }

            Start_Worker_Shuttle(i, worker_per_bus, day, offset);
            offset += worker_per_bus;
            sleep(1); // Add a small delay to ensure signal handling is completed
        }
        exit(0);
    } else if (pid > 0) { // Parent process
        // No need to set up the signal handler in the parent process
        int status;
        waitpid(pid, &status, 0);
    } else {
        printf("Error: Fork failed\n");
    }

    // Call the Menu() function here, after the child process has finished
    Menu();
}

int main()
{
    Menu();
    return 0;
}

void Menu() {
    int ch = 6;
    do {
        printf("\n***************************************************");
        printf("\nMain Menu - Drink My Friend ");
        printf("\n***************************************************");
        printf("\n1. Add Applicant");
        printf("\n2. Modify Applicant");
        printf("\n3. Delete Applicant");
        printf("\n4. View the List of All Applicants");
        printf("\n5. View the List of Applicants by Day");
        printf("\n6. Start Worker Shuttle");
        printf("\n7. Exit");
        printf("\n***************************************************");
        printf("\nPlease enter your option: ");
        scanf("%d", &ch);

        switch (ch) {
            case 1:
                New_Applicant();
                break;
            case 2:
                Modify_Applicant();
                break;
            case 3:
                Delete_Applicant();
                break;
            case 4:
                Display_List();
                break;
            case 5:
                Display_By_Day();
                break;
            case 6:
                {
                    int choice;
                    char day[10];
                    bool valid_day;

                    do {
                        printf("Enter the day of the week to start workers (e.g., monday): ");
                        scanf("%s", day);

                        valid_day = strcmp(day, "monday") == 0 || strcmp(day, "tuesday") == 0 || strcmp(day, "wednesday") == 0 ||
                                    strcmp(day, "thursday") == 0 || strcmp(day, "friday") == 0;

                        if (!valid_day) {
                            printf("Invalid day, please enter a valid day.\n");
                        }
                    } while (!valid_day);

                    Start_Workers(day);
                }
                break;
            case 7:
                exit(0);
                break;
            default:
                printf("\nERROR: The option with this number does not exist.\nPlease choose another option.");
                break;
        }
    } while (ch != 7);
}

#define MAX_MONDAY_WORKERS 10
#define MAX_WEDNESDAY_WORKERS 10
#define MAX_THURSDAY_WORKERS 10
#define MAX_TUESDAY_WORKERS 10
#define MAX_FRIDAY_WORKERS 10

bool check_day_availability(const char *days);

void New_Applicant() {
    applicant a;
    FILE *f;
    f = fopen("applicants.txt", "a");

    printf("Enter Applicant ID: ");
    scanf("%d", &a.id);
    printf("Enter Applicant Name: ");
    scanf("%s", a.name);
    printf("Enter the number of days the applicant is available (max %d): ", MAX_DAYS);
    scanf("%d", &a.num_days);

    // Initialize the days array with null terminators
    for (int i = 0; i < MAX_DAYS; i++) {
        a.days[i][0] = '\0';
    }

    char days[MAX_DAYS][10];
    printf("Enter the days the applicant is available (separated by space): ");
    for (int i = 0; i < a.num_days; i++) {
        scanf("%s", days[i]);
    }

    // Check if any of the input days has reached its maximum worker limit
    bool can_add_applicant = true;
    for (int i = 0; i < a.num_days; i++) {
        if (!check_day_availability(days[i])) {
            printf("Cannot add applicant: The maximum number of workers for %s has been reached.\n", days[i]);
            can_add_applicant = false;
            break;
        }
    }

    // Add the applicant if all the input days are available
    if (can_add_applicant) {
        for (int i = 0; i < a.num_days; i++) {
            strcpy(a.days[i], days[i]);
        }
        fwrite(&a, sizeof(applicant), 1, f);
        printf("\nSYSTEM: Applicant has been added.");
    }

    fclose(f);
}

bool check_day_availability(const char *days) {
    int monday_count = 0, wednesday_count = 0, thursday_count = 0, tuesday_count = 0, friday_count = 0;
    applicant a;
    FILE *f;

    f = fopen("applicants.txt", "r");
    if (f == NULL) {
        printf("\nError opening file.");
        return false;
    }

    while (fread(&a, sizeof(a), 1, f)) {
        for (int i = 0; i < MAX_DAYS; i++) {
            if (strstr(a.days[i], "monday")) {
                monday_count++;
            }
            if (strstr(a.days[i], "tuesday")) {
                tuesday_count++;
            }
            if (strstr(a.days[i], "wednesday")) {
                wednesday_count++;
            }
            if (strstr(a.days[i], "thursday")) {
                thursday_count++;
            }
            if (strstr(a.days[i], "friday")) {
                friday_count++;
            }
        }
    }

    fclose(f);

    if ((strstr(days, "monday") && monday_count >= MAX_MONDAY_WORKERS) ||
        (strstr(days, "tuesday") && tuesday_count >= MAX_TUESDAY_WORKERS) ||
        (strstr(days, "wednesday") && wednesday_count >= MAX_WEDNESDAY_WORKERS) ||
        (strstr(days, "thursday") && thursday_count >= MAX_THURSDAY_WORKERS) ||
        (strstr(days, "friday") && friday_count >= MAX_FRIDAY_WORKERS)) {
        return false;
    } else {
        return true;
    }
}

void Modify_Applicant() {
    applicant a;
    FILE *f, *f2;
    int id, temp = 0;
    char temp_days[50];
    char *token;
    int i = 0;

    f = fopen("applicants.txt", "r");
    f2 = fopen("temp.txt", "w");

    printf("\nEnter the ID of your application to Modify: ");
    scanf("%d", &id);
    getchar(); // consume newline character from buffer

    while (fread(&a, sizeof(applicant), 1, f)) {
        if (a.id == id) {
            temp = 1;
            printf("\n\nPlease enter the updated information.");

            printf("\n\nNew ID\t: ");
            scanf("%d", &a.id);
            getchar(); // consume newline character from buffer

            printf("New name\t: ");
            fgets(a.name, sizeof(a.name), stdin);
            a.name[strcspn(a.name, "\n")] = '\0'; // remove newline character

            printf("Choose New Days (separated by space): (monday, tuesday, wednesday, thursday, friday)\t: ");
            fgets(temp_days, sizeof(temp_days), stdin);
            temp_days[strcspn(temp_days, "\n")] = '\0'; // remove newline character

            token = strtok(temp_days, " ");
            i = 0;
            while (token != NULL && i < MAX_DAYS) {
                strncpy(a.days[i], token, sizeof(a.days[i]) - 1);
                a.days[i][sizeof(a.days[i]) - 1] = '\0';
                token = strtok(NULL, " ");
                i++;
            }

            if (!check_day_availability(a.days)) {
                printf("\nOne or more selected days are already full. Applicant not modified.");
                return;
            }
        }
        fwrite(&a, sizeof(applicant), 1, f2);
    }
    fclose(f);
    fclose(f2);

    if (temp) {
        f2 = fopen("temp.txt", "r");
        f = fopen("applicants.txt", "w");
        while (fread(&a, sizeof(applicant), 1, f2)) {
            fwrite(&a, sizeof(applicant), 1, f);
        }
        fclose(f);
        fclose(f2);
        printf("\nSYSTEM: The Application has been modified successfully.");
        return;
    } else
        printf("\nERROR: The Applicant ID does not exist.");
}


void Delete_Applicant()   // delete an applicant
{
    applicant a;
    FILE *f, *fD;
    int id, tempD=0;
    f = fopen("applicants.txt", "r");
    fD = fopen("tempD.txt", "w");

    printf("\nEnter the ID of your Application to Delete: ");
    scanf("%d", &id);

    while(fread(&a, sizeof(applicant), 1, f))
    {
        if(a.id == id)
        {
            tempD = 1;
        }
        else
            fwrite(&a, sizeof(applicant), 1, fD);
    }
    fclose(f);
    fclose(fD);

    if(tempD){
        fD = fopen("tempD.txt", "r");
        f = fopen("applicants.txt", "w");
        while(fread(&a, sizeof(applicant), 1, fD))
        {
            fwrite(&a, sizeof(applicant), 1, f);
        }
        fclose(f);
        fclose(fD);
        printf("\nSYSTEM: The Application has been deleted succesfully.");
    }
    else
        printf("\nERROR: The Applicant ID does not exist.");
}


void Display_List() {
    applicant a;
    FILE *f;
    f = fopen("applicants.txt", "r");
    while (fread(&a, sizeof(applicant), 1, f)) {
        printf("\n%5d %10s", a.id, a.name);
        for (int i = 0; i < MAX_DAYS; i++) {
            if (strlen(a.days[i]) > 0) {
                printf(" %10s", a.days[i]);
            }
        }
        printf("\n");
    }
    fclose(f);
    printf("\nSYSTEM: All Applicants have been listed.");
}

void Display_By_Day() {
    applicant a;
    FILE *f;
    char day[50];
    f = fopen("applicants.txt", "r");

    printf("\nEnter the Day of the Applicants to List (monday, tuesday, wednesday, thursday, friday): ");
    scanf("%s", day);

    while (fread(&a, sizeof(applicant), 1, f)) {
        for (int i = 0; i < MAX_DAYS; i++) {
            if (strcmp(a.days[i], day) == 0) {
                printf("\n%5d %10s\n", a.id, a.name);
                break; // break the inner loop as the day is found
            }
        }
    }
    fclose(f);
    printf("\nSYSTEM: All applicants from this day are listed.");
}
