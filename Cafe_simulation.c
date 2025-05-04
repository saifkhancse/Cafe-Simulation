#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <pthread.h>

#include <semaphore.h>

#include <unistd.h>

#include <time.h>

//// Cafe Start
typedef struct {
  char Book_Name[200];
  int Book_Status;
}
Book;

typedef struct {
  int Table_Number;
  int Table_Status;
}
Table;

typedef struct {
  int Table;
  int Book[3];
  int no_of_book;
}
Customer;

#define MAX_BOOK 10
#define MAX_TABLE 5
#define MAX_Customer 1000

int Sleep_time = 1;

//// Critical Section Start
Table Tables[MAX_TABLE];
int Total_Table = 0;
Book Book_Inventory[MAX_BOOK];
int Total_Book = 0;
Customer All_Customers[MAX_Customer];
int Total_Customer = 0;
//// Critical Section End

//// Semaphores & Mutex Start
sem_t tableSemaphore;
pthread_mutex_t TableMutex;
pthread_mutex_t CustomerMutex;
pthread_mutex_t BookMutex;
sem_t table_done_sem[MAX_Customer], book_assign_done[MAX_Customer];
//// Semaphores & Mutex End

//// Prototyping Start
// Loading functions
void Load_Book_Inventory(int n);
void Load_Table(int n);
void Table_Book_Leave();

// Showing functions
void Show_Book_Inventory();
void Show_Table();
void Show_All_Customer();
void Show_Menu();

// Thread Functions
void * Adding_Table_Thread(void * arg);
void * Load_Book_Inventory_Thread(void * arg);
void * Show_Table_Thread(void * arg);
void * Show_Book_Inventory_Thread(void * arg);
void * Show_Customer_Info_Thread(void * arg);
void * Load_Customer_Table_Thread(void * arg);
void * Load_Customer_Book_Thread(void * arg);
void * Unassign_Table_Thread(void * arg);

//// Prototyping End

//// Adding Start
// Table
void * Adding_Table_Thread(void * arg) {
  sem_wait( & tableSemaphore);
  int index = Total_Table;
  Tables[index].Table_Number = index + 1;
  Tables[index].Table_Status = 1;
  Total_Table++;
  sem_post( & tableSemaphore);
  pthread_exit(NULL);
}
void Load_Table(int n) {
  pthread_t threads[n];
  for (int i = 0; i < n; i++) {
    if (pthread_create( & threads[i], NULL, Adding_Table_Thread, NULL) != 0) {
      printf("Failed to create thread %d\n", i);
    }
  }
  for (int i = 0; i < n; i++) {
    pthread_join(threads[i], NULL);
  }
  sem_destroy( & tableSemaphore);
  printf("|-----------------------------------|\n");
  printf("|-----%-2d Tables Loaded in total-----|\n", Total_Table);
  printf("|-----------------------------------|\n");
  fflush(stdout);
}
// Book
void * Load_Book_Inventory_Thread(void * arg) {
  int i = * (int * ) arg;
  pthread_mutex_lock( & BookMutex);
  sprintf(Book_Inventory[i].Book_Name, "Book %2d", i + 1);
  Book_Inventory[i].Book_Status = 1;
  Total_Book++;
  pthread_mutex_unlock( & BookMutex);
  return NULL;
}
void Load_Book_Inventory(int n) {
  pthread_t bthreads[n];
  for (int i = 0; i < n; i++) {
    int * index = malloc(sizeof(int));
    * index = i;
    pthread_create( & bthreads[i], NULL, Load_Book_Inventory_Thread, index);
  }
  for (int i = 0; i < n; i++) {
    pthread_join(bthreads[i], NULL);
  }
  printf("|-----------------------------------|\n");
  printf("|------%-2d Books Loaded in total-----|\n", Total_Book);
  printf("|-----------------------------------|\n");
  fflush(stdout);
}
//// Adding End

//// Showing Start

// Table

void Show_Table() {
  printf("|-----------------------------------|\n");
  printf("|-------------Tables----------------|\n");
  for (int i = 0; i < Total_Table; i++) {
    printf("Table %d:", Tables[i].Table_Number);
    if (Tables[i].Table_Status == 1) {
      printf("Available |");
    } else {
      printf("Occupied  |");
    }
    if ((i + 1) % 2 == 0) printf("\n");
  }
  printf("\n|-----------------------------------|\n");
  fflush(stdout);
}

// Book
void Show_Book_Inventory() {
    pthread_mutex_lock( & BookMutex);
printf("|-----------------------------------|\n");
  for (int i = 0; i < Total_Book; i++) {

printf("|%s: ", Book_Inventory[i].Book_Name);
  if (Book_Inventory[i].Book_Status == 1) {
    printf("Available");
  } else {
    printf("Unvailable");
  }
if((i+1)%2==0)printf("\n");
  }
printf("|-----------------------------------|\n");
  pthread_mutex_unlock( & BookMutex);
}
// Customer

void * Show_Customer_Info_Thread(void * arg) {
  int i = * (int * ) arg;
  pthread_mutex_lock( & CustomerMutex);
  pthread_mutex_lock( & BookMutex);
  if (All_Customers[i].Table == 0) {
    pthread_mutex_unlock( & BookMutex);
    pthread_mutex_unlock( & CustomerMutex);
    return NULL;
  }
  printf("Customer %d is occupying Table %d and ", i + 1, All_Customers[i].Table);
  if (All_Customers[i].no_of_book == 0) {
    printf("not reading anything.\n");
  } else {
    printf("reading ");
    for (int j = 0; j < All_Customers[i].no_of_book; j++) {
      printf("%s", Book_Inventory[All_Customers[i].Book[j]].Book_Name);
      if (j < All_Customers[i].no_of_book - 1) {
        printf(", ");
      }
    }
    printf(".\n");
  }
  pthread_mutex_unlock( & BookMutex);
  pthread_mutex_unlock( & CustomerMutex);
  return NULL;
}

void Show_All_Customer() {
  pthread_t sthreads[Total_Customer];
  for (int i = 0; i < Total_Customer; i++) {
    pthread_mutex_lock( & CustomerMutex);
    pthread_mutex_lock( & BookMutex);
    if (All_Customers[i].Table == 0) {
      pthread_mutex_unlock( & BookMutex);
      pthread_mutex_unlock( & CustomerMutex);
      return;
    }
    printf("Customer %d is occupying Table %d and ", i + 1, All_Customers[i].Table);
    if (All_Customers[i].no_of_book == 0) {
      printf("not reading anything.\n");
    } else {
      printf("reading ");
      for (int j = 0; j < All_Customers[i].no_of_book; j++) {
        printf("%s", Book_Inventory[All_Customers[i].Book[j]].Book_Name);
        if (j < All_Customers[i].no_of_book - 1) {
          printf(", ");
        }
      }
      printf(".\n");
    }
    pthread_mutex_unlock( & BookMutex);
    pthread_mutex_unlock( & CustomerMutex);
  }
}

//// Showing End

/////////////////////////////////////////////////////
void * Load_Customer_Table_Thread(void * arg) {
  int i = * ((int * ) arg);
  printf("|-----------------------------------|\n");
  printf("|--Customer %d searching for table--| \n", i);
  printf("|-----------------------------------|\n");
  while (1) {
    sem_wait( & tableSemaphore);
    pthread_mutex_lock( & TableMutex);
    pthread_mutex_lock( & CustomerMutex);

    int table_found = 0;

    for (int j = 0; j < Total_Table; j++) {
      if (Tables[j].Table_Status == 1) {
        Tables[j].Table_Status = 0;
        All_Customers[i].Table = Tables[j].Table_Number;
        All_Customers[i].no_of_book = 0;
        Total_Customer++;
        table_found = 1;
        printf("|-----------------------------------|\n");
        printf("|--Table %d assigned to Customer %d-| \n", All_Customers[i].Table, i);
        printf("|-----------------------------------|\n");
        pthread_mutex_unlock( & TableMutex);
        pthread_mutex_unlock( & CustomerMutex);
        sem_post( & tableSemaphore);
        sem_post( & table_done_sem[i]);

        sleep(Sleep_time);
        return NULL;
      }
    }

    pthread_mutex_unlock( & TableMutex);
    pthread_mutex_unlock( & CustomerMutex);
    sem_post( & tableSemaphore);

    if (!table_found) {

      printf("|-----------------------------------|\n");
      printf("|-No table available at the moment--|\n");
      printf("|---Customer %d waiting in queue----|\n", i);
      printf("|-----------------------------------|\n");
      fflush(stdout);

      sleep(Sleep_time);
    }
  }
}
void * Load_Customer_Book_Thread(void * arg) {
  int i = * (int * ) arg;
  sem_wait( & table_done_sem[i]);
  pthread_mutex_lock( & CustomerMutex);
  pthread_mutex_lock( & BookMutex);
  printf("|-----------------------------------|\n");
  printf("|------Customer %d picking books-----|\n", i);
  int num_books_to_assign = rand() % 3 + 1;
  int books_assigned = 0;
  for (int j = 0; j < Total_Book && books_assigned < num_books_to_assign; j++) {
    if (Book_Inventory[j].Book_Status == 1) {
      Book_Inventory[j].Book_Status = 0;
      All_Customers[i].Book[books_assigned] = j;
      books_assigned++;
      printf("|-----Customer %d picked %s------|\n", i, Book_Inventory[j].Book_Name);
      printf("|-----------------------------------|\n");
      fflush(stdout);
      sleep(Sleep_time);
    }
  }

  All_Customers[i].no_of_book = books_assigned;
  pthread_mutex_unlock( & CustomerMutex);
  pthread_mutex_unlock( & BookMutex);
  sem_post( & book_assign_done[i]);
  return NULL;
}

void * Unassign_Table_Thread(void * arg) {
  int i = * (int * ) arg;
  sem_wait( & book_assign_done[i]);
  sleep(Sleep_time);
  pthread_mutex_lock( & TableMutex);
  pthread_mutex_lock( & CustomerMutex);
  for (int j = 0; j < Total_Table; j++) {
    if (Tables[j].Table_Number == All_Customers[i].Table) {
      Tables[j].Table_Status = 1;
      break;
    }
  }

  for (int j = 0; j < All_Customers[i].no_of_book; j++) {
    int bookIndex = All_Customers[i].Book[j];
    Book_Inventory[bookIndex].Book_Status = 1;
  }
  //Show_All_Customer();
  printf("|-----------------------------------|\n");
  printf("|---Customer %d Enjoyed his Treats---|\n", i);
  if (All_Customers[i].no_of_book != 0) {
    printf("|--------and his Reading time!------|\n");
  }
  printf("|-------Now leaving his Table %d-----|\n", All_Customers[i].Table);
  printf("|-----------------------------------|\n");
  fflush(stdout);
  All_Customers[i].Table = 0;
  All_Customers[i].no_of_book = 0;
  sleep(Sleep_time);
  pthread_mutex_unlock( & TableMutex);
  pthread_mutex_unlock( & CustomerMutex);
  free(arg);
  Show_Table();
Show_Book_Inventory();

  return NULL;
}

void Table_Book_Leave() {
  int n;
  printf("Enter number of customers to load: ");
  scanf("%d", & n);
  system("clear");
  printf("|-----------------------------------|\n");
  printf("|------Welcome To W&L's Cafe--------|\n");
  printf("|-----------------------------------|\n");
  printf("|-----------------------------------|\n");
  printf("|-------Simulation Started----------|\n");
  printf("|------%d Customers In queue!-------|\n", n);
  printf("|-----------------------------------|\n");
  printf("|-----------------------------------|\n");
  fflush(stdout);

  pthread_t TableThread[n];
  pthread_t BookThread[n];
  pthread_t LeaveThread[n];
int temp=Total_Customer;
  for (int i = temp, j = 0; i <temp+ n; i++, j++) {
    int * arg1 = malloc(sizeof(int));
    int * arg2 = malloc(sizeof(int));
    int * arg3 = malloc(sizeof(int));
    * arg1 = i;
    * arg2 = i;
    * arg3 = i;
    pthread_create( & TableThread[j], NULL, Load_Customer_Table_Thread, arg1);
    pthread_create( & BookThread[j], NULL, Load_Customer_Book_Thread, arg2);
    pthread_create( & LeaveThread[j], NULL, Unassign_Table_Thread, arg3);
  }

  for (int i = 0; i < n; i++) {
    pthread_join(TableThread[i], NULL);
    pthread_join(BookThread[i], NULL);
    pthread_join(LeaveThread[i], NULL);
  }

}

///////////////////////////////////
//// Cafe End
void Show_Menu() {
  int choice;
  while (1) {
    printf("|-----------------------------------|\n");
    printf("|------Welcome To W&L's Cafe--------|\n");
    printf("|-----------------------------------|\n");
    printf("|--------------Choose---------------|\n");
    printf("|------1. Load Customers to Tables--|\n");
    printf("|------2. Exit----------------------|\n");
    printf("|------Enter your choice: ");
    fflush(stdout);
    scanf("%d", & choice);
    switch (choice) {
    case 1:
      Table_Book_Leave();
      break;
    case 2:
      printf("|-----------------------------------|\n");
      printf("|----%2d Customer Visited Today-----|\n",Total_Customer);
      printf("|-----------------------------------|\n");
      printf("|------Cafe Closed For Today!-------|\n");
      printf("|-----------------------------------|\n");
      fflush(stdout);
      return;
    default:
      system("clear");
      printf("|-----------------------------------|\n");
      printf("|-Invalid choice. Please try again--|\n");
      printf("|-----------------------------------|\n");
      fflush(stdout);
      break;
    }
  }
}

int main() {
  sem_init( & tableSemaphore, 0, 1);

  for (int i = 0; i < MAX_Customer; i++) {
    sem_init( & table_done_sem[i], 0, 0);
    sem_init( & book_assign_done[i], 0, 0);
  }
  Load_Book_Inventory(MAX_BOOK);
  Load_Table(MAX_TABLE);
  Show_Menu();

}