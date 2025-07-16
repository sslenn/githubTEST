#include "FinalProject220200yesyes.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <windows.h> // For Sleep on Windows

// ================= GLOBAL VARIABLES =================
StockItem inventory[MAX_ITEMS];
int stockCount = 0;
int isAdmin = 0;

// ================= FILE OPERATION PROTOTYPES =================
void saveToFile(void);
void loadFromFile(void);

// ================= SAFE INPUT HELPERS =================
void readLine(char *buffer, int size) {
    if (fgets(buffer, size, stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0';
    }
}
int isValidInt(const char *str) {
    if (*str == '\0' || isspace(*str)) return 0;
    char *endptr;
    strtol(str, &endptr, 10);
    return *endptr == '\0';
}
int isValidFloat(const char *str) {
    if (*str == '\0' || isspace(*str)) return 0;
    char *endptr;
    strtof(str, &endptr);
    return *endptr == '\0';
}
int inputInt(const char *prompt) {
    char buffer[100];
    int value;
    while (1) {
        printf("%s", prompt);
        fflush(stdout); // Ensure prompt is shown before input
        readLine(buffer, sizeof(buffer));
        if (isValidInt(buffer)) {
            value = (int)strtol(buffer, NULL, 10);
            return value;
        } else {
            printf("Invalid input. Please enter a valid number.\n");
        }
    }
}
float inputFloat(const char *prompt) {
    char buffer[100];
    float value;
    while (1) {
        printf("%s", prompt);
        fflush(stdout); // Ensure prompt is shown before input
        readLine(buffer, sizeof(buffer));
        if (isValidFloat(buffer)) {
            value = strtof(buffer, NULL);
            return value;
        } else {
            printf("Invalid input. Please enter a valid number.\n");
        }
    }
}

// ================= LOGIN SYSTEM =================

void login() {
    int roleChoice = inputInt("Select role:\n1. Admin\n2. User\nEnter choice (1-2): ");
    if (roleChoice == 1) {
        int loginSuccess = 0;
        do {
            char username[20], password[20];
            printf("Enter admin username: ");
            readLine(username, sizeof(username));
            printf("Enter admin password: ");
            readLine(password, sizeof(password));
            if (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0) {
                isAdmin = 1;
                printf("Admin login successful.\n");
                loginSuccess = 1;
            } else {
                printf("Invalid admin credentials. Please try again.\n");
            }
        } while (!isAdmin);
    } else {
        printf("Logged in as Customer.\n");
    }
}

// ================= DATE VALIDATION =================

int isValidDate(int day, int month, int year) {
    if (month < 1 || month > 12) return 0;
    if (year < 2020 || year > 2070) return 0; // Ensure year is 4 digits
    int daysInMonth[] = {31, 28, 31, 30, 31, 30,
                         31, 31, 30, 31, 30, 31};

    // Leap year check
    if (month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) {
        daysInMonth[1] = 29;
    }

    if (day < 1 || day > daysInMonth[month - 1]) return 0;

    return 1;
}

Date getToday() {
    Date today;
    int valid = 0;
    do {
        today.day = inputInt("Enter today's date (DD): ");
        today.month = inputInt("Enter today's date (MM): ");
        today.year = inputInt("Enter today's date (YYYY): ");
        if (!isValidDate(today.day, today.month, today.year)) {
            printf("Invalid date. Please enter a valid day, month, and 4-digit year (YYYY) between 2020 and 2070.\n");
        } else {
            valid = 1;
        }
    } while (!valid);
    return today;
}

int isExpired(Date expiry, Date today) {
    if (expiry.year < today.year) return 1;
    if (expiry.year == today.year && expiry.month < today.month) return 1;
    if (expiry.year == today.year && expiry.month == today.month && expiry.day < today.day) return 1;
    return 0;
}

// ================= NAME VALIDATION =================
int isValidName(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isalpha(str[i]) && !isspace(str[i])) {
            return 0;
        }
    }
    return 1;
}

// ================= CATEGORY SELECTION HELPER =================
const char *selectCategory() {
    static const char *categories[] = {
        "Food and Beverage",
        "Household Product",
        "Personal Care",
        "Health"
    };
    int catChoice = inputInt("Select Category:\n1. Food and Beverage\n2. Household Product\n3. Personal Care\n4. Health\nEnter choice (1-4): ");
    if (catChoice >= 1 && catChoice <= 4) {
        return categories[catChoice - 1];
    }
    return "Uncategorized";
}

// ================= INVENTORY ACTIONS =================

void addStock() {
    if (stockCount >= MAX_ITEMS) {
        printf("Inventory full!\n");
        return;
    }
    spinnerAnimation("Preparing to add new stock", 8, 60);
    StockItem item;
    item.id = inputInt("Enter Stock ID: ");
    for (int i = 0; i < stockCount; i++) {
        if (inventory[i].id == item.id) {
            printf("Stock ID already exists. Try again.\n");
            return;
        }
    }
    char nameBuffer[50];
    do {
        printf("Enter Stock Name: ");
        fflush(stdout);
        readLine(nameBuffer, sizeof(nameBuffer));
        if (!isValidName(nameBuffer) || strlen(nameBuffer) == 0) {
            printf("Invalid name. Please use only letters and spaces.\n");
        }
    } while (!isValidName(nameBuffer) || strlen(nameBuffer) == 0);
    strncpy(item.name, nameBuffer, sizeof(item.name));
    item.name[sizeof(item.name)-1] = '\0';
    strncpy(item.category, selectCategory(), sizeof(item.category));
    item.category[sizeof(item.category)-1] = '\0';
    item.quantity = inputInt("Enter Quantity: ");
    if (item.quantity < 0) {
        printf("Quantity cannot be negative.\n");
        return;
    }
    item.price = inputFloat("Enter Price per Unit: ");
    if (item.price < 0) {
        printf("Price cannot be negative.\n");
        return;
    }
    do {
        item.expiry.day = inputInt("Enter Expiry Day: ");
        item.expiry.month = inputInt("Enter Expiry Month: ");
        item.expiry.year = inputInt("Enter Expiry Year (YYYY): ");
        if (!isValidDate(item.expiry.day, item.expiry.month, item.expiry.year)) {
            printf("Invalid date. Please enter a valid day, month, and 4-digit year (YYYY).\n");
        }
    } while (!isValidDate(item.expiry.day, item.expiry.month, item.expiry.year));
    spinnerAnimation("Adding stock", 10, 40);
    inventory[stockCount++] = item;
    printf("Stock added successfully.\n");
}

void viewStock() {
    spinnerAnimation("Loading stock list", 12, 30);
    if (stockCount == 0) {
        printf("No stock available.\n");
        return;
    }
    printf("\n%-5s %-20s %-20s %-10s %-10s %-15s\n", "ID", "Name", "Category", "Quantity", "Price", "Expiry Date");
    for (int i = 0; i < stockCount; i++) {
        printf("%-5d %-20s %-20s %-10d $%-10.2f %02d/%02d/%04d\n",
               inventory[i].id, inventory[i].name, inventory[i].category,
               inventory[i].quantity, inventory[i].price,
               inventory[i].expiry.day, inventory[i].expiry.month, inventory[i].expiry.year);
    }
}

void modifyStock() {
    int id = inputInt("Enter Stock ID to modify: ");
    int found = 0;
    for (int i = 0; i < stockCount; i++) {
        if (inventory[i].id == id) {
            spinnerAnimation("Fetching stock info", 8, 60);
            found = 1;
            printf("\nCurrent Info:\n");
            printf("%-5s %-20s %-20s %-10s %-10s %-15s\n", "ID", "Name", "Category", "Quantity", "Price", "Expiry Date");
            printf("%-5d %-20s %-20s %-10d $%-10.2f %02d/%02d/%04d\n",
                inventory[i].id, inventory[i].name, inventory[i].category, inventory[i].quantity, inventory[i].price,
                inventory[i].expiry.day, inventory[i].expiry.month, inventory[i].expiry.year);
            printf("\nWhich info do you want to change?\n");
            printf("1. Name\n2. Category\n3. Quantity\n4. Price\n5. Expiry Date\n6. Cancel\n");
            int opt = inputInt("Select option (1-6): ");
            char buffer[100];
            switch (opt) {
                case 1:
                    printf("New Name [%s]: ", inventory[i].name);
                    readLine(buffer, sizeof(buffer));
                    if (strlen(buffer) > 0 && isValidName(buffer)) {
                        strncpy(inventory[i].name, buffer, sizeof(inventory[i].name));
                        inventory[i].name[sizeof(inventory[i].name)-1] = '\0';
                        printf("Name updated.\n");
                    } else if (strlen(buffer) > 0) {
                        printf("Invalid name. No changes made.\n");
                    }
                    break;
                case 2:
                    strncpy(inventory[i].category, selectCategory(), sizeof(inventory[i].category));
                    inventory[i].category[sizeof(inventory[i].category)-1] = '\0';
                    printf("Category updated.\n");
                    break;
                case 3:
                    printf("New Quantity [%d]: ", inventory[i].quantity);
                    readLine(buffer, sizeof(buffer));
                    if (strlen(buffer) > 0 && isValidInt(buffer)) {
                        int qty = atoi(buffer);
                        if (qty >= 0) inventory[i].quantity = qty;
                        else printf("Quantity cannot be negative. Keeping previous value.\n");
                        printf("Quantity updated.\n");
                    }
                    break;
                case 4:
                    printf("New Price [%.2f]: ", inventory[i].price);
                    readLine(buffer, sizeof(buffer));
                    if (strlen(buffer) > 0 && isValidFloat(buffer)) {
                        float price = strtof(buffer, NULL);
                        if (price >= 0) inventory[i].price = price;
                        else printf("Price cannot be negative. Keeping previous value.\n");
                        printf("Price updated.\n");
                    }
                    break;
                case 5:
                    printf("New Expiry Day [%02d]: ", inventory[i].expiry.day);
                    readLine(buffer, sizeof(buffer));
                    if (strlen(buffer) > 0 && isValidInt(buffer)) inventory[i].expiry.day = atoi(buffer);
                    printf("New Expiry Month [%02d]: ", inventory[i].expiry.month);
                    readLine(buffer, sizeof(buffer));
                    if (strlen(buffer) > 0 && isValidInt(buffer)) inventory[i].expiry.month = atoi(buffer);
                    printf("New Expiry Year [%04d]: ", inventory[i].expiry.year);
                    readLine(buffer, sizeof(buffer));
                    if (strlen(buffer) > 0 && isValidInt(buffer)) inventory[i].expiry.year = atoi(buffer);
                    if (!isValidDate(inventory[i].expiry.day, inventory[i].expiry.month, inventory[i].expiry.year)) {
                        printf("Invalid expiry date entered. Keeping previous expiry date.\n");
                    } else {
                        printf("Expiry date updated.\n");
                    }
                    break;
                case 6:
                    printf("Modification cancelled.\n");
                    break;
                default:
                    printf("Invalid option. No changes made.\n");
            }
            break;
        }
    }
    if (!found) {
        printf("Stock ID not found.\n");
    }
}

void removeStock() {
    int id = inputInt("Enter Stock ID to remove: ");
    int found = 0;
    for (int i = 0; i < stockCount; i++) {
        if (inventory[i].id == id) {
            spinnerAnimation("Removing stock", 10, 40);
            for (int j = i; j < stockCount - 1; j++) {
                inventory[j] = inventory[j + 1];
            }
            stockCount--;
            printf("Stock removed.\n");
            found = 1;
            break;
        }
    }
    if (!found) {
        printf("Stock ID not found.\n");
    }
}

void showExpired() {
    spinnerAnimation("Checking for expired stock", 10, 40);
    Date today = getToday();
    int found = 0;

    printf("\nExpired Stock:\n");
    printf("%-5s %-20s %-20s %-10s %-15s\n", "ID", "Name", "Category", "Qty", "Expiry");

    for (int i = 0; i < stockCount; i++) {
        if (isExpired(inventory[i].expiry, today)) {
            printf("%-5d %-20s %-20s %-10d %02d/%02d/%04d\n",
                   inventory[i].id,
                   inventory[i].name,
                   inventory[i].category,
                   inventory[i].quantity,
                   inventory[i].expiry.day,
                   inventory[i].expiry.month,
                   inventory[i].expiry.year);
            found = 1;
        }
    }

    if (!found) {
        printf("No expired stock.\n");
    }
}

void summaryReport() {
    spinnerAnimation("Generating summary report", 12, 30);
    float totalValue = 0;
    printf("\n%-5s %-20s %-20s %-10s %-15s\n", "ID", "Name", "Category", "Quantity", "Total Price");
    for (int i = 0; i < stockCount; i++) {
        float productTotal = inventory[i].quantity * inventory[i].price;
        printf("%-5d %-20s %-20s %-10d $%-14.2f\n",
            inventory[i].id, inventory[i].name, inventory[i].category, inventory[i].quantity, productTotal);
        totalValue += productTotal;
    }
    printf("\nTotal Price of All Stock: $%.2f\n", totalValue);
}

void searchStock() {
    spinnerAnimation("Searching stock", 10, 40);
    char query[50];
    printf("Enter product name or category to search: ");
    readLine(query, sizeof(query));
    int found = 0;
    printf("\n%-5s %-20s %-20s %-10s %-10s %-15s\n", "ID", "Name", "Category", "Quantity", "Price", "Expiry Date");
    for (int i = 0; i < stockCount; i++) {
        if (strstr(inventory[i].name, query) || strstr(inventory[i].category, query)) {
            printf("%-5d %-20s %-20s %-10d $%-10.2f %02d/%02d/%04d\n",
                inventory[i].id, inventory[i].name, inventory[i].category, inventory[i].quantity, inventory[i].price,
                inventory[i].expiry.day, inventory[i].expiry.month, inventory[i].expiry.year);
            found = 1;
        }
    }
    if (!found) printf("No matching stock found.\n");
}

// ================= MENU =================

void menu() {
    int choice;
    do {
        printf("\n=========== Stock Management Menu (%s) ===========\n", isAdmin ? "Admin" : "Customer");
        if (isAdmin) {
            printf("1. Add New Stock\n");
            printf("2. View All Stock\n");
            printf("3. Modify Stock Info\n");
            printf("4. Remove Stock\n");
            printf("5. Show Expired Stock\n");
            printf("6. Inventory Summary Report\n");
            printf("7. Save Inventory to File\n");
            printf("8. Load Inventory from File\n");
            printf("9. Search Stock\n");
            printf("10. Exit\n");
            choice = inputInt("Select option: ");
            switch (choice) {
                case 1: addStock(); break;
                case 2: viewStock(); break;
                case 3: modifyStock(); break;
                case 4: removeStock(); break;
                case 5: showExpired(); break;
                case 6: summaryReport(); break;
                case 7: saveToFile(); break;
                case 8: loadFromFile(); break;
                case 9: searchStock(); break;
                case 10: printf("Exiting... Thank you!\n"); break;
                default: printf("Invalid option. Try again.\n");
            }
        } else {
            printf("1. View All Stock\n");
            printf("2. Show Expired Stock\n");
            printf("3. Inventory Summary Report\n");
            printf("4. Search Stock\n");
            printf("5. Exit\n");
            choice = inputInt("Select option: ");
            switch (choice) {
                case 1: viewStock(); break;
                case 2: showExpired(); break;
                case 3: summaryReport(); break;
                case 4: searchStock(); break;
                case 5: printf("Exiting... Thank you!\n"); break;
                default: printf("Invalid option. Try again.\n");
            }
        }
    } while ((isAdmin && choice != 10) || (!isAdmin && choice != 5));
}

// ================= FILE OPERATIONS =================

void saveToFile() {
    loadingAnimation("Saving inventory", 3, 200);
    FILE *fp = fopen(FILE_NAME, "w");
    if (fp == NULL) {
        printf("Error opening file for saving.\n");
        return;
    }
    for (int i = 0; i < stockCount; i++) {
        fprintf(fp, "%d\n%s\n%s\n%d\n%.2f\n%d %d %d\n",
            inventory[i].id,
            inventory[i].name,
            inventory[i].category,
            inventory[i].quantity,
            inventory[i].price,
            inventory[i].expiry.day,
            inventory[i].expiry.month,
            inventory[i].expiry.year);
    }
    fclose(fp);
    printf("Inventory saved successfully.\n");
}

void loadFromFile() {
    loadingAnimation("Loading inventory", 3, 200);
    FILE *fp = fopen(FILE_NAME, "r");
    if (fp == NULL) {
        printf("No saved inventory found. Starting fresh.\n");
        return;
    }
    stockCount = 0;
    while (fscanf(fp, "%d\n", &inventory[stockCount].id) == 1) {
        fgets(inventory[stockCount].name, sizeof(inventory[stockCount].name), fp);
        inventory[stockCount].name[strcspn(inventory[stockCount].name, "\n")] = '\0';
        fgets(inventory[stockCount].category, sizeof(inventory[stockCount].category), fp);
        inventory[stockCount].category[strcspn(inventory[stockCount].category, "\n")] = '\0';
        fscanf(fp, "%d\n%f\n%d %d %d\n",
            &inventory[stockCount].quantity,
            &inventory[stockCount].price,
            &inventory[stockCount].expiry.day,
            &inventory[stockCount].expiry.month,
            &inventory[stockCount].expiry.year);
        stockCount++;
        if (stockCount >= MAX_ITEMS) break;
    }
    fclose(fp);
    printf("Loaded %d items from %s.\n", stockCount, FILE_NAME);
}

// ================= MAIN =================

int main() {
    loadingAnimation("Starting Stock Management System", 5, 200);
    printf("Welcome to the Stock Management System\n");
    login();
    loadFromFile();
    menu();
    return 0;
}

void loadingAnimation(const char *message, int dots, int delayMs) {
    printf("%s", message);
    fflush(stdout);
    for (int i = 0; i < dots; i++) {
        Sleep(delayMs);
        printf(".");
        fflush(stdout);
    }
    printf("\n");
}

void spinnerAnimation(const char *message, int cycles, int delayMs) {
    const char spinner[] = "|/-\\";
    printf("%s ", message);
    fflush(stdout);
    for (int i = 0; i < cycles; i++) {
        printf("%c\b", spinner[i % 4]);
        fflush(stdout);
        Sleep(delayMs);
    }
    printf(" \n");
}

