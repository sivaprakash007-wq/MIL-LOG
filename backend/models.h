#ifndef MODELS_H
#define MODELS_H

#define MAX_UNITS 100
#define MAX_EQUIPMENT 200
#define MAX_REQUESTS 200

typedef struct {
    int id;
    char name[50];
    char location[50];
    int personnel_count;
} Unit;

typedef struct {
    int id;
    char name[50];
    char category[50];
    int quantity;
    int available;
} Equipment;

typedef struct {
    int id;
    int unit_id;
    int equipment_id;
    int quantity;
    int priority;
    int burst_time;
    int memory_required;
    int status;
} SupplyRequest;

extern Unit units[MAX_UNITS];
extern Equipment equipment[MAX_EQUIPMENT];
extern SupplyRequest requests[MAX_REQUESTS];

extern int unit_count;
extern int equipment_count;
extern int request_count;

#endif