#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

// ================= FUNCTION PROTOTYPES =================
void loadingAnimation(const char *message, int dots, int delayMs);
void spinnerAnimation(const char *message, int cycles, int delayMs);

// ================= CONSTANTS =================
#define MAX_ITEMS 100
#define FILE_NAME "inventory.txt"

// ================= STRUCTURES =================
typedef struct {
    int day;
    int month;
    int year;
} Date;

typedef struct {
    int id;
    char name[50];
    char category[50];
    int quantity;
    float price;
    Date expiry;
} StockItem;

// ================= GLOBAL VARIABLES =================
StockItem inventory[MAX_ITEMS];
int stockCount = 0;
int isAdmin = 0;

// ================= SAFE INPUT HELPERS =================

void readLine(char *buffer, int size) {
    if (fgets(buffer, size, stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0';
    }
}

int isValidInt(const char *str) {
    if (*str == '\0' || isspace(*str)) return 0;
    if (strcmp(str, "cancel") == 0) return 1;
    char *endptr;
    strtol(str, &endptr, 10);
    return *endptr == '\0';
}

int isValidFloat(const char *str) {
    if (*str == '\0' || isspace(*str)) return 0;
    if (strcmp(str, "cancel") == 0) return 1;
    char *endptr;
    errno = 0;
    strtof(str, &endptr);
    return *endptr == '\0' && errno == 0;
}

int inputInt(const char *prompt, const char *section) {
    char buffer[100];
    if (section) {
        printf("\n---- %s ----\n", section);
        printf("Type 'cancel' to exit\n");
    }
    while (1) {
        printf("%s", prompt);
        fflush(stdout);
        readLine(buffer, sizeof(buffer));
        if (strcmp(buffer, "cancel") == 0) return -1;
        if (isValidInt(buffer)) {
            return (int)strtol(buffer, NULL, 10);
        }
        printf("*** Invalid input! Please enter a valid number. ***\n");
    }
}

float inputFloat(const char *prompt, const char *section) {
    char buffer[100];
    if (section) {
        printf("\n---- %s ----\n", section);
        printf("Type 'cancel' to exit\n");
    }
    while (1) {
        printf("%s", prompt);
        fflush(stdout);
        readLine(buffer, sizeof(buffer));
        if (strcmp(buffer, "cancel") == 0) return -1.0f;
        if (isValidFloat(buffer)) {
            float value = strtof(buffer, NULL);
            if (errno == 0) return value;
        }
        printf("*** Invalid input! Please enter a valid number. ***\n");
    }
}

// ================= LOGIN SYSTEM =================

void login() {
    printf("\n==================================================\n");
    printf("                 Login to Stock Management        \n");
    printf("==================================================\n");
    while (1) {
        int roleChoice = inputInt("Select Role:\n1. Admin\n2. User\nEnter choice (1-2): ", "Role Selection");
        if (roleChoice == -1) {
            printf("*** Login cancelled. Exiting system. ***\n");
            exit(0);
        }
        if (roleChoice == 1) {
            while (1) {
                char username[20], password[20];
                printf("\n---- Admin Login ----\n");
                printf("Type 'cancel' to exit\n");
                printf("Enter admin username: ");
                readLine(username, sizeof(username));
                if (strcmp(username, "cancel") == 0) {
                    printf("*** Login cancelled. Returning to role selection. ***\n\n");
                    break;
                }
                printf("Enter admin password: ");
                readLine(password, sizeof(password));
                if (strcmp(password, "cancel") == 0) {
                    printf("*** Login cancelled. Returning to role selection. ***\n\n");
                    break;
                }
                if (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0) {
                    isAdmin = 1;
                    printf("\n*** Admin login successful! ***\n\n");
                    return;
                }
                printf("*** Invalid admin credentials! Please try again. ***\n\n");
            }
        } else if (roleChoice == 2) {
            printf("\n*** Logged in as Customer. ***\n\n");
            return;
        } else {
            printf("*** Invalid choice! Please enter 1 or 2. ***\n\n");
        }
    }
}

// ================= DATE VALIDATION =================

int isValidDate(int day, int month, int year) {
    if (month < 1 || month > 12) return 0;
    if (year < 2020 || year > 2070) return 0;
    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) {
        daysInMonth[1] = 29;
    }
    if (day < 1 || day > daysInMonth[month - 1]) return 0;
    return 1;
}

Date getToday() {
    Date today;
    printf("\n---- Enter Today's Date ----\n");
    printf("Type 'cancel' to exit\n");
    while (1) {
        today.day = inputInt("Enter day (DD): ", NULL);
        if (today.day == -1) {
            printf("*** Date entry cancelled. Using default date (01/01/2020). ***\n\n");
            today.day = 1;
            today.month = 1;
            today.year = 2020;
            return today;
        }
        today.month = inputInt("Enter month (MM): ", NULL);
        if (today.month == -1) {
            printf("*** Date entry cancelled. Using default date (01/01/2020). ***\n\n");
            today.day = 1;
            today.month = 1;
            today.year = 2020;
            return today;
        }
        today.year = inputInt("Enter year (YYYY): ", NULL);
        if (today.year == -1) {
            printf("*** Date entry cancelled. Using default date (01/01/2020). ***\n\n");
            today.day = 1;
            today.month = 1;
            today.year = 2020;
            return today;
        }
        if (isValidDate(today.day, today.month, today.year)) {
            return today;
        }
        printf("*** Invalid date! Please enter a valid day (1-31), month (1-12), and year (2020-2070). ***\n");
    }
}

int isExpired(Date expiry, Date today) {
    if (expiry.year < today.year) return 1;
    if (expiry.year == today.year && expiry.month < today.month) return 1;
    if (expiry.year == today.year && expiry.month == today.month && expiry.day < today.day) return 1;
    return 0;
}

// ================= NAME VALIDATION =================

int isValidName(const char *str) {
    if (strlen(str) == 0) return 0;
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
        "Health & Medicine"
    };
    while (1) {
        int catChoice = inputInt("1. Food and Beverage\n2. Household Product\n3. Personal Care\n4. Health & Medicine\nEnter choice (1-4): ", "Select Category");
        if (catChoice == -1) {
            printf("*** Category selection cancelled. Using 'Uncategorized'. ***\n\n");
            return "Uncategorized";
        }
        if (catChoice >= 1 && catChoice <= 4) {
            return categories[catChoice - 1];
        }
        printf("*** Invalid choice! Please enter a number between 1 and 4. ***\n");
    }
}

// ================= INVENTORY ACTIONS =================

void addStock() {
    if (stockCount >= MAX_ITEMS) {
        printf("\n*** Inventory full! Cannot add more items. ***\n\n");
        return;
    }
    printf("\n====================================\n");
    printf("       Add New Stock Item      \n");
    printf("====================================\n");
    spinnerAnimation("Preparing to add new stock", 8, 60);

    StockItem item;
    while (1) {
        int isDuplicate = 0;
        item.id = inputInt("Enter Stock ID (1-1000): ", "Stock ID");
        if (item.id == -1) {
            printf("*** Stock addition cancelled. ***\n\n");
            return;
        }
        if (item.id < 1 || item.id > 1000) {
            printf("*** Invalid Stock ID! Must be between 1 and 1000. ***\n");
            continue;
        }
        for (int i = 0; i < stockCount; i++) {
            if (inventory[i].id == item.id) {
                printf("*** Stock ID already exists! Please enter a different ID. ***\n");
                isDuplicate = 1;
                break;
            }
        }
        if (!isDuplicate) break;
    }

    char nameBuffer[50];
    printf("\n---- Stock Name ----\n");
    printf("Type 'cancel' to exit\n");
    while (1) {
        printf("Enter Stock Name: ");
        fflush(stdout);
        readLine(nameBuffer, sizeof(nameBuffer));
        if (strcmp(nameBuffer, "cancel") == 0) {
            printf("*** Stock addition cancelled. ***\n\n");
            return;
        }
        if (isValidName(nameBuffer)) {
            strncpy(item.name, nameBuffer, sizeof(item.name));
            item.name[sizeof(item.name) - 1] = '\0';
            break;
        }
        printf("*** Invalid name! Please use only letters and spaces. ***\n");
    }

    const char *category = selectCategory();
    if (strcmp(category, "Uncategorized") == 0 && item.id == -1) {
        printf("*** Stock addition cancelled due to category selection exit. ***\n\n");
        return;
    }
    strncpy(item.category, category, sizeof(item.category));
    item.category[sizeof(item.category) - 1] = '\0';

    item.quantity = inputInt("Enter Quantity (1-99999): ", "Quantity");
    if (item.quantity == -1) {
        printf("*** Stock addition cancelled. ***\n\n");
        return;
    }
    while (item.quantity < 1 || item.quantity > 99999) {
        printf("*** Quantity must be between 1 and 99999! ***\n");
        item.quantity = inputInt("Enter Quantity (1-99999): ", NULL);
        if (item.quantity == -1) {
            printf("*** Stock addition cancelled. ***\n\n");
            return;
        }
    }

    item.price = inputFloat("Enter Price per Unit (0.1-99999.99): ", "Price");
    if (item.price == -1.0f) {
        printf("*** Stock addition cancelled. ***\n\n");
        return;
    }
    while (item.price < 0.1 || item.price > 99999.99) {
        printf("*** Price must be between 0.1 and 99999.99! ***\n");
        item.price = inputFloat("Enter Price per Unit (0.1-99999.99): ", NULL);
        if (item.price == -1.0f) {
            printf("*** Stock addition cancelled. ***\n\n");
            return;
        }
    }

    item.expiry.day = inputInt("Enter Expiry Day (1-31): ", "Expiry Date");
    if (item.expiry.day == -1) {
        printf("*** Stock addition cancelled. ***\n\n");
        return;
    }
    while (item.expiry.day < 1 || item.expiry.day > 31) {
        printf("*** Invalid day! Please enter a day between 1 and 31. ***\n");
        item.expiry.day = inputInt("Enter Expiry Day (1-31): ", NULL);
        if (item.expiry.day == -1) {
            printf("*** Stock addition cancelled. ***\n\n");
            return;
        }
    }

    item.expiry.month = inputInt("Enter Expiry Month (1-12): ", NULL);
    if (item.expiry.month == -1) {
        printf("*** Stock addition cancelled. ***\n\n");
        return;
    }
    while (item.expiry.month < 1 || item.expiry.month > 12) {
        printf("*** Invalid month! Please enter a month between 1 and 12. ***\n");
        item.expiry.month = inputInt("Enter Expiry Month (1-12): ", NULL);
        if (item.expiry.month == -1) {
            printf("*** Stock addition cancelled. ***\n\n");
            return;
        }
    }

    item.expiry.year = inputInt("Enter Expiry Year (2024-2100): ", NULL);
    if (item.expiry.year == -1) {
        printf("*** Stock addition cancelled. ***\n\n");
        return;
    }
    while (item.expiry.year < 2024 || item.expiry.year > 2100) {
        printf("*** Invalid year! Please enter a year between 2024 and 2100. ***\n");
        item.expiry.year = inputInt("Enter Expiry Year (2024-2100): ", NULL);
        if (item.expiry.year == -1) {
            printf("*** Stock addition cancelled. ***\n\n");
            return;
        }
    }

    while (!isValidDate(item.expiry.day, item.expiry.month, item.expiry.year)) {
        printf("*** Invalid date combination! Please re-enter the date: ***\n");
        item.expiry.day = inputInt("Enter Expiry Day (1-31): ", "Re-enter Expiry Date");
        if (item.expiry.day == -1) {
            printf("*** Stock addition cancelled. ***\n\n");
            return;
        }
        item.expiry.month = inputInt("Enter Expiry Month (1-12): ", NULL);
        if (item.expiry.month == -1) {
            printf("*** Stock addition cancelled. ***\n\n");
            return;
        }
        item.expiry.year = inputInt("Enter Expiry Year (2024-2100): ", NULL);
        if (item.expiry.year == -1) {
            printf("*** Stock addition cancelled. ***\n\n");
            return;
        }
    }

    spinnerAnimation("Adding stock", 10, 40);
    inventory[stockCount++] = item;
    printf("\n*** Stock added successfully! ***\n\n");
}

void viewStock() {
    printf("\n====================================\n");
    printf("       View All Stock Items      \n");
    printf("====================================\n");
    spinnerAnimation("Loading stock list", 12, 30);
    if (stockCount == 0) {
        printf("\n*** No stock available. ***\n\n");
        return;
    }
    printf("\n%-5s %-20s %-20s %-10s %-10s %-15s\n", "ID", "Name", "Category", "Quantity", "Price", "Expiry Date");
    printf("----------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < stockCount; i++) {
        printf("%-5d %-20s %-20s %-10d $%-10.2f %02d/%02d/%04d\n",
               inventory[i].id, inventory[i].name, inventory[i].category,
               inventory[i].quantity, inventory[i].price,
               inventory[i].expiry.day, inventory[i].expiry.month, inventory[i].expiry.year);
    }
    printf("----------------------------------------------------------------------------------------------------\n\n");
}

void modifyStock() {
    printf("\n====================================\n");
    printf("       Modify Stock Item      \n");
    printf("====================================\n");
    while (1) {
        int id = inputInt("Enter Stock ID to modify (1-1000): ", "Modify Stock");
        if (id == -1) {
            printf("*** Modification cancelled. ***\n\n");
            return;
        }
        if (id < 1 || id > 1000) {
            printf("*** Invalid Stock ID! Must be between 1 and 1000. ***\n");
            continue;
        }
        int found = 0;
        for (int i = 0; i < stockCount; i++) {
            if (inventory[i].id == id) {
                found = 1;
                spinnerAnimation("Fetching stock info", 8, 60);
                printf("\n---- Current Stock Info ----\n");
                printf("%-5s %-20s %-20s %-10s %-10s %-15s\n", "ID", "Name", "Category", "Quantity", "Price", "Expiry Date");
                printf("---------------------------------------------------------------------------------------------------\n");
                printf("%-5d %-20s %-20s %-10d $%-10.2f %02d/%02d/%04d\n",
                       inventory[i].id, inventory[i].name, inventory[i].category,
                       inventory[i].quantity, inventory[i].price,
                       inventory[i].expiry.day, inventory[i].expiry.month, inventory[i].expiry.year);
                printf("---------------------------------------------------------------------------------------------------\n");
                while (1) {
                    printf("\n---- Modify Options ----\n");
                    printf("Type 'cancel' to exit\n");
                    int opt = inputInt("1. Name\n2. Category\n3. Quantity\n4. Price\n5. Expiry Date\n6. Cancel\nSelect option (1-6): ", NULL);
                    if (opt == -1 || opt == 6) {
                        printf("*** Modification cancelled. ***\n\n");
                        return;
                    }
                    char buffer[100];
                    char prompt[100];
                    switch (opt) {
                        case 1:
                            printf("\n---- Modify Name ----\n");
                            printf("Type 'cancel' to exit\n");
                            while (1) {
                                snprintf(prompt, sizeof(prompt), "New Name [%s]: ", inventory[i].name);
                                printf("%s", prompt);
                                readLine(buffer, sizeof(buffer));
                                if (strcmp(buffer, "cancel") == 0) {
                                    printf("*** Name update cancelled. ***\n\n");
                                    return;
                                }
                                if (strlen(buffer) > 0 && isValidName(buffer)) {
                                    strncpy(inventory[i].name, buffer, sizeof(inventory[i].name));
                                    inventory[i].name[sizeof(inventory[i].name) - 1] = '\0';
                                    printf("\n*** Name updated successfully! ***\n\n");
                                    return;
                                }
                                printf("*** Invalid name! Please use only letters and spaces. ***\n");
                            }
                        case 2:
                            strncpy(inventory[i].category, selectCategory(), sizeof(inventory[i].category));
                            inventory[i].category[sizeof(inventory[i].category) - 1] = '\0';
                            printf("\n*** Category updated successfully! ***\n\n");
                            return;
                        case 3:
                            printf("\n---- Modify Quantity ----\n");
                            printf("Type 'cancel' to exit\n");
                            while (1) {
                                snprintf(prompt, sizeof(prompt), "New Quantity [%d] (0-99999): ", inventory[i].quantity);
                                int qty = inputInt(prompt, NULL);
                                if (qty == -1) {
                                    printf("*** Quantity update cancelled. ***\n\n");
                                    return;
                                }
                                if (qty >= 0 && qty <= 99999) {
                                    inventory[i].quantity = qty;
                                    printf("\n*** Quantity updated successfully! ***\n\n");
                                    return;
                                }
                                printf("*** Quantity must be between 0 and 99999! ***\n");
                            }
                        case 4:
                            printf("\n---- Modify Price ----\n");
                            printf("Type 'cancel' to exit\n");
                            while (1) {
                                snprintf(prompt, sizeof(prompt), "New Price [%.2f] (0-99999.99): ", inventory[i].price);
                                float price = inputFloat(prompt, NULL);
                                if (price == -1.0f) {
                                    printf("*** Price update cancelled. ***\n\n");
                                    return;
                                }
                                if (price >= 0 && price <= 99999.99) {
                                    inventory[i].price = price;
                                    printf("\n*** Price updated successfully! ***\n\n");
                                    return;
                                }
                                printf("*** Price must be between 0 and 99999.99! ***\n");
                            }
                        case 5:
                            printf("\n---- Modify Expiry Date ----\n");
                            printf("Type 'cancel' to exit\n");
                            int newDay, newMonth, newYear;
                            while (1) {
                                snprintf(prompt, sizeof(prompt), "New Expiry Day [%02d]: ", inventory[i].expiry.day);
                                newDay = inputInt(prompt, NULL);
                                if (newDay == -1) {
                                    printf("*** Expiry date update cancelled. ***\n\n");
                                    return;
                                }
                                if (newDay >= 1 && newDay <= 31) break;
                                printf("*** Invalid day! Please enter a day between 1 and 31. ***\n");
                            }
                            while (1) {
                                snprintf(prompt, sizeof(prompt), "New Expiry Month [%02d]: ", inventory[i].expiry.month);
                                newMonth = inputInt(prompt, NULL);
                                if (newMonth == -1) {
                                    printf("*** Expiry date update cancelled. ***\n\n");
                                    return;
                                }
                                if (newMonth >= 1 && newMonth <= 12) break;
                                printf("*** Invalid month! Please enter a month between 1 and 12. ***\n");
                            }
                            while (1) {
                                snprintf(prompt, sizeof(prompt), "New Expiry Year [%04d]: ", inventory[i].expiry.year);
                                newYear = inputInt(prompt, NULL);
                                if (newYear == -1) {
                                    printf("*** Expiry date update cancelled. ***\n\n");
                                    return;
                                }
                                if (newYear >= 2024 && newYear <= 2100) break;
                                printf("*** Invalid year! Please enter a year between 2024 and 2100. ***\n");
                            }
                            if (isValidDate(newDay, newMonth, newYear)) {
                                inventory[i].expiry.day = newDay;
                                inventory[i].expiry.month = newMonth;
                                inventory[i].expiry.year = newYear;
                                printf("\n*** Expiry date updated successfully! ***\n\n");
                                return;
                            }
                            printf("*** Invalid expiry date! No changes made. ***\n\n");
                            return;
                        default:
                            printf("*** Invalid option! Please select 1-6. ***\n");
                    }
                }
            }
        }
        if (!found) {
            printf("*** Stock ID not found! Please try again. ***\n");
        }
    }
}

void removeStock() {
    printf("\n====================================\n");
    printf("       Remove Stock Item      \n");
    printf("====================================\n");
    while (1) {
        int id = inputInt("Enter Stock ID to remove (1-1000): ", "Remove Stock");
        if (id == -1) {
            printf("*** Stock removal cancelled. ***\n\n");
            return;
        }
        if (id < 1 || id > 1000) {
            printf("*** Invalid Stock ID! Must be between 1 and 1000. ***\n");
            continue;
        }
        for (int i = 0; i < stockCount; i++) {
            if (inventory[i].id == id) {
                spinnerAnimation("Removing stock", 10, 40);
                for (int j = i; j < stockCount - 1; j++) {
                    inventory[j] = inventory[j + 1];
                }
                stockCount--;
                printf("\n*** Stock removed successfully! ***\n\n");
                return;
            }
        }
        printf("*** Stock ID not found! Please try again. ***\n");
    }
}

void showExpired() {
    printf("\n====================================\n");
    printf("       Expired Stock Items      \n");
    printf("====================================\n");
    spinnerAnimation("Checking for expired stock", 10, 40);
    Date today = getToday();
    int found = 0;

    printf("\n%-5s %-20s %-20s %-10s %-15s\n", "ID", "Name", "Category", "Qty", "Expiry");
    printf("---------------------------------------------------------------------------\n");
    for (int i = 0; i < stockCount; i++) {
        if (isExpired(inventory[i].expiry, today)) {
            printf("%-5d %-20s %-20s %-10d %02d/%02d/%04d\n",
                   inventory[i].id, inventory[i].name, inventory[i].category,
                   inventory[i].quantity, inventory[i].expiry.day,
                   inventory[i].expiry.month, inventory[i].expiry.year);
            found = 1;
        }
    }
    printf("--------------------------------------------------------------------------\n");
    if (!found) {
        printf("\n*** No expired stock found. ***\n\n");
    } else {
        printf("\n");
    }
}

void summaryReport() {
    printf("\n====================================\n");
    printf("       Inventory Summary Report      \n");
    printf("=====================================\n");
    spinnerAnimation("Generating summary report", 12, 30);
    float totalValue = 0;
    printf("\n%-5s %-20s %-20s %-10s %-15s\n", "ID", "Name", "Category", "Quantity", "Total Price");
    printf("-------------------------------------------------------------------------------------\n");
    for (int i = 0; i < stockCount; i++) {
        float productTotal = inventory[i].quantity * inventory[i].price;
        printf("%-5d %-20s %-20s %-10d $%-14.2f\n",
               inventory[i].id, inventory[i].name, inventory[i].category,
               inventory[i].quantity, productTotal);
        totalValue += productTotal;
    }
    printf("------------------------------------------------------------------------------------\n");
    printf("\nTotal Price of All Stock: $%.2f\n\n", totalValue);
}

void toLowerStr(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

void searchStock() {
    printf("\n====================================\n");
    printf("       Search Stock Items      \n");
    printf("====================================\n");
    spinnerAnimation("Searching stock", 10, 40);
    char query[50];
    printf("\n---- Search Stock ----\n");
    printf("Type 'cancel' to exit\n");
    while (1) {
        printf("Enter product name or ID to search: ");
        readLine(query, sizeof(query));
        if (strcmp(query, "cancel") == 0) {
            printf("*** Search cancelled. ***\n\n");
            return;
        }
        if (strlen(query) > 0) break;
        printf("*** Invalid input! Please enter a name or ID. ***\n");
    }

    char lowerQuery[50];
    strcpy(lowerQuery, query);
    toLowerStr(lowerQuery);

    int found = 0;
    printf("\n%-5s %-20s %-20s %-10s %-10s %-15s\n", "ID", "Name", "Category", "Qty", "Price", "Expiry");
    printf("-------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < stockCount; i++) {
        char lowerName[100];
        strcpy(lowerName, inventory[i].name);
        toLowerStr(lowerName);
        if ((strstr(lowerName, lowerQuery) != NULL) || (isValidInt(query) && inventory[i].id == atoi(query))) {
            printf("%-5d %-20s %-20s %-10d $%-10.2f %02d/%02d/%04d\n",
                   inventory[i].id, inventory[i].name, inventory[i].category,
                   inventory[i].quantity, inventory[i].price,
                   inventory[i].expiry.day, inventory[i].expiry.month, inventory[i].expiry.year);
            found = 1;
        }
    }
    printf("-------------------------------------------------------------------------------------------\n");
    if (!found) {
        printf("\n*** No matching stock found. ***\n\n");
    } else {
        printf("\n");
    }
}

void searchStockByCategoryOrId() {
    printf("\n====================================\n");
    printf("  Search Stock by Category or ID  \n");
    printf("====================================\n");
    spinnerAnimation("Preparing search", 10, 40);

    const char *categories[] = {
        "Food and Beverage",
        "Household Product",
        "Personal Care",
        "Health & Medicine"
    };
    const char *selectedCategory = selectCategory();
    if (strcmp(selectedCategory, "Uncategorized") == 0) {
        printf("*** Search cancelled. ***\n\n");
        return;
    }

    int id = -1; // Default to no ID filter
    char idBuffer[100];
    printf("\n---- Stock ID ----\n");
    printf("Type 'cancel' to exit or press Enter to search by category only\n");
    while (1) {
        printf("Enter Stock ID (1-1000) or press Enter: ");
        readLine(idBuffer, sizeof(idBuffer));
        if (strcmp(idBuffer, "cancel") == 0 || idBuffer[0] == '\0') {
            break; // No ID filter
        }
        if (isValidInt(idBuffer)) {
            id = (int)strtol(idBuffer, NULL, 10);
            if (id >= 1 && id <= 1000) {
                break;
            }
            printf("*** Invalid Stock ID! Must be between 1 and 1000. ***\n");
        } else {
            printf("*** Invalid input! Please enter a valid number or press Enter. ***\n");
        }
    }

    int found = 0;
    printf("\n---- Search Results for Category '%s'%s ----\n", selectedCategory, id != -1 ? " or Specified ID" : "");
    printf("%-5s %-20s %-20s %-10s %-10s %-15s\n", "ID", "Name", "Category", "Qty", "Price", "Expiry");
    printf("-----------------------------------------------------------------------------------------\n");
    for (int i = 0; i < stockCount; i++) {
        if (strcmp(inventory[i].category, selectedCategory) == 0 || (id != -1 && inventory[i].id == id)) {
            printf("%-5d %-20s %-20s %-10d $%-10.2f %02d/%02d/%04d\n",
                   inventory[i].id, inventory[i].name, inventory[i].category,
                   inventory[i].quantity, inventory[i].price,
                   inventory[i].expiry.day, inventory[i].expiry.month, inventory[i].expiry.year);
            found = 1;
        }
    }
    printf("-----------------------------------------------------------------------------------------\n");
    if (!found) {
        printf("\n*** No matching stock found for category '%s'%s. ***\n\n",
               selectedCategory, id != -1 ? " or specified ID" : "");
    } else {
        printf("\n");
    }
}

// ================= FILE OPERATIONS =================

void saveToFile() {
    printf("\n====================================\n");
    printf("       Saving Inventory      \n");
    printf("====================================\n");
    spinnerAnimation("Saving inventory", 10, 40);
    FILE *fp = fopen(FILE_NAME, "w");
    if (fp == NULL) {
        printf("\n*** Error opening file for saving! ***\n\n");
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
    printf("\n*** Inventory saved successfully to %s! ***\n\n", FILE_NAME);
}

void loadFromFile() {
    printf("\n====================================\n");
    printf("       Loading Inventory      \n");
    printf("====================================\n");
    spinnerAnimation("Loading inventory", 10, 40);
    FILE *fp = fopen(FILE_NAME, "r");
    if (fp == NULL) {
        printf("\n*** No saved inventory found. Starting fresh. ***\n\n");
        return;
    }
    stockCount = 0;
    while (fscanf(fp, "%d\n", &inventory[stockCount].id) == 1) {
        if (inventory[stockCount].id < 1 || inventory[stockCount].id > 1000) {
            char buffer[100];
            fgets(buffer, sizeof(buffer), fp);
            fgets(buffer, sizeof(buffer), fp);
            fscanf(fp, "%*d\n%*f\n%*d %*d %*d\n");
            continue;
        }
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
        if (inventory[stockCount].quantity < 0 || inventory[stockCount].quantity > 99999 ||
            inventory[stockCount].price < 0 || inventory[stockCount].price > 99999.99 ||
            !isValidDate(inventory[stockCount].expiry.day, inventory[stockCount].expiry.month, inventory[stockCount].expiry.year)) {
            continue;
        }
        stockCount++;
        if (stockCount >= MAX_ITEMS) break;
    }
    fclose(fp);
    printf("\n*** Loaded %d valid items from %s! ***\n\n", stockCount, FILE_NAME);
}

// ================= MENU =================

void menu() {
    int choice;
    do {
        printf("\n====================================\n");
        printf("   Stock Management System (%s)   \n", isAdmin ? "Admin" : "Customer");
        printf("====================================\n");
        if (isAdmin) {
            printf("1. Add New Stock\n");
            printf("2. View All Stock\n");
            printf("3. Modify Stock Info\n");
            printf("4. Remove Stock\n");
            printf("5. Show Expired Stock\n");
            printf("6. Inventory Summary Report\n");
            printf("7. Save Inventory to File\n"); 
            printf("8. Search Stock by Category or ID\n");
            printf("9. Exit\n");
            printf("------------------------------------\n");
            choice = inputInt("Select option (1-9): ", "Main Menu");
            switch (choice) {
                case 1: addStock(); break;
                case 2: viewStock(); break;
                case 3: modifyStock(); break;
                case 4: removeStock(); break;
                case 5: showExpired(); break;
                case 6: summaryReport(); break;
                case 7: saveToFile(); break;
                case 8: searchStockByCategoryOrId(); break;
                case 9: printf("\n*** Exiting... Thank you! ***\n\n"); break;
                default: printf("\n*** Invalid option! Please select 1-9. ***\n\n");
            }
        } else {
            printf("1. View All Stock\n");
            printf("2. Show Expired Stock\n");
            printf("3. Inventory Summary Report\n");
            printf("4. Search Stock by Category or ID\n");
            printf("5. Exit\n");
            printf("------------------------------------\n");
            choice = inputInt("Select option (1-5): ", "Main Menu");
            switch (choice) {
                case 1: viewStock(); break;
                case 2: showExpired(); break;
                case 3: summaryReport(); break;
                case 4: searchStockByCategoryOrId(); break;
                case 5: printf("\n*** Exiting... Thank you! ***\n\n"); break;
                default: printf("\n*** Invalid option! Please select 1-5. ***\n\n");
            }
        }
    } while ((isAdmin && choice != 9) || (!isAdmin && choice != 5));
}

// ================= MAIN =================

int main() {
    printf("\n====================================================================\n");
    printf("\n                    Welcome To WAREWISE                             \n");
    printf("======================================================================\n");
    spinnerAnimation("Starting system", 8, 60);
    login();
    loadFromFile();
    menu();
    return 0;
}

// ================= ANIMATIONS =================

void loadingAnimation(const char *message, int dots, int delayMs) {
    printf("%s", message);
    fflush(stdout);
    for (int i = 0; i < dots; i++) {
        usleep(delayMs * 2000); // Convert milliseconds to microseconds
        printf(".");
        fflush(stdout);
    }
    printf("\n");
}

void spinnerAnimation(const char *message, int cycles, int delayMs) {
    const char spinner[] = "|/-\\";
    printf("---- %s ", message);
    fflush(stdout);
    for (int i = 0; i < cycles; i++) {
        printf("%c\b", spinner[i % 4]);
        fflush(stdout);
        usleep(delayMs * 2000); // Convert milliseconds to microseconds
    }
    printf("\n");
}