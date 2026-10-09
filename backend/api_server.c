
#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "models.h"
#include "units.h"
#include "inventory.h"
#include "requests.h"
#include "storage.h"
#include "filesystem.h"

#define PORT 8090
#define BUFFER_SIZE 65536
#define MAX_BODY 32768

typedef struct {
    int id;
    char name[50];
    int size;
} ApiFileRecord;

static ApiFileRecord api_files[50];
static int api_file_count = 0;

static void load_api_files(void) {
    FILE *f = openDataFile("files.dat", "rb");
    if (!f) return;
    if (fread(&api_file_count, sizeof(int), 1, f) != 1 ||
        api_file_count < 0 || api_file_count > 50) {
        api_file_count = 0;
        fclose(f);
        return;
    }
    if (fread(api_files, sizeof(ApiFileRecord), (size_t)api_file_count, f)
        != (size_t)api_file_count) {
        api_file_count = 0;
    }
    fclose(f);
}

static void save_api_files(void) {
    FILE *f = openDataFile("files.dat", "wb");
    if (!f) return;
    fwrite(&api_file_count, sizeof(int), 1, f);
    fwrite(api_files, sizeof(ApiFileRecord), (size_t)api_file_count, f);
    fclose(f);
}

static void json_escape(const char *src, char *dst, size_t n) {
    size_t j = 0;
    for (size_t i = 0; src[i] && j + 2 < n; ++i) {
        unsigned char c = (unsigned char)src[i];
        if (c == '"' || c == '\\') {
            if (j + 2 >= n) break;
            dst[j++] = '\\';
            dst[j++] = (char)c;
        } else if (c == '\n') {
            dst[j++] = '\\'; dst[j++] = 'n';
        } else if (c == '\r') {
            dst[j++] = '\\'; dst[j++] = 'r';
        } else if (c >= 32) {
            dst[j++] = (char)c;
        }
    }
    dst[j] = '\0';
}

static void send_json(int fd, int status, const char *body) {
    const char *status_text = status == 200 ? "OK" :
                              status == 201 ? "Created" :
                              status == 400 ? "Bad Request" :
                              status == 404 ? "Not Found" : "Internal Server Error";
    char header[512];
    int len = (int)strlen(body);
    int h = snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Cache-Control: no-store\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n\r\n",
        status, status_text, len);
    send(fd, header, (size_t)h, 0);
    send(fd, body, (size_t)len, 0);
}

static void send_error(int fd, int status, const char *message) {
    char body[512], esc[256];
    json_escape(message, esc, sizeof(esc));
    snprintf(body, sizeof(body), "{\"ok\":false,\"error\":\"%s\"}", esc);
    send_json(fd, status, body);
}

static int json_int(const char *body, const char *key, int *out) {
    char needle[64];
    snprintf(needle, sizeof(needle), "\"%s\"", key);
    const char *p = strstr(body, needle);
    if (!p) return 0;
    p = strchr(p, ':');
    if (!p) return 0;
    ++p;
    while (isspace((unsigned char)*p)) ++p;
    char *end;
    long v = strtol(p, &end, 10);
    if (end == p) return 0;
    *out = (int)v;
    return 1;
}

static int json_string(const char *body, const char *key, char *out, size_t out_size) {
    char needle[64];
    snprintf(needle, sizeof(needle), "\"%s\"", key);
    const char *p = strstr(body, needle);
    if (!p) return 0;
    p = strchr(p, ':');
    if (!p) return 0;
    ++p;
    while (isspace((unsigned char)*p)) ++p;
    if (*p != '"') return 0;
    ++p;
    size_t j = 0;
    while (*p && *p != '"' && j + 1 < out_size) {
        if (*p == '\\' && p[1]) ++p;
        out[j++] = *p++;
    }
    out[j] = '\0';
    return *p == '"';
}

static int next_id_units(void) {
    int max = 0;
    for (int i = 0; i < unit_count; ++i) if (units[i].id > max) max = units[i].id;
    return max + 1;
}

static int next_id_equipment(void) {
    int max = 0;
    for (int i = 0; i < equipment_count; ++i)
        if (equipment[i].id > max) max = equipment[i].id;
    return max + 1;
}

static int next_id_request(void) {
    int max = 0;
    for (int i = 0; i < request_count; ++i)
        if (requests[i].id > max) max = requests[i].id;
    return max + 1;
}

static int next_id_file(void) {
    int max = 0;
    for (int i = 0; i < api_file_count; ++i)
        if (api_files[i].id > max) max = api_files[i].id;
    return max + 1;
}

static void get_units(int fd) {
    char body[20000];
    size_t used = 0;
    used += (size_t)snprintf(body + used, sizeof(body) - used, "{\"ok\":true,\"units\":[");
    for (int i = 0; i < unit_count; ++i) {
        char n[128], l[128];
        json_escape(units[i].name, n, sizeof(n));
        json_escape(units[i].location, l, sizeof(l));
        used += (size_t)snprintf(body + used, sizeof(body) - used,
            "%s{\"id\":%d,\"name\":\"%s\",\"location\":\"%s\",\"personnel\":%d}",
            i ? "," : "", units[i].id, n, l, units[i].personnel_count);
    }
    snprintf(body + used, sizeof(body) - used, "]}");
    send_json(fd, 200, body);
}

static void get_inventory(int fd) {
    char body[30000];
    size_t used = 0;
    used += (size_t)snprintf(body + used, sizeof(body) - used, "{\"ok\":true,\"inventory\":[");
    for (int i = 0; i < equipment_count; ++i) {
        char n[128], c[128];
        json_escape(equipment[i].name, n, sizeof(n));
        json_escape(equipment[i].category, c, sizeof(c));
        used += (size_t)snprintf(body + used, sizeof(body) - used,
            "%s{\"id\":%d,\"name\":\"%s\",\"category\":\"%s\",\"quantity\":%d,\"available\":%d}",
            i ? "," : "", equipment[i].id, n, c, equipment[i].quantity, equipment[i].available);
    }
    snprintf(body + used, sizeof(body) - used, "]}");
    send_json(fd, 200, body);
}

static const char *status_name(int s) {
    if (s == 0) return "Pending";
    if (s == 1) return "Completed";
    return "Cancelled";
}

static void get_requests(int fd) {
    char body[30000];
    size_t used = 0;
    used += (size_t)snprintf(body + used, sizeof(body) - used, "{\"ok\":true,\"requests\":[");
    for (int i = 0; i < request_count; ++i) {
        used += (size_t)snprintf(body + used, sizeof(body) - used,
            "%s{\"id\":%d,\"unit\":%d,\"equipment\":%d,\"quantity\":%d,"
            "\"priority\":%d,\"burst\":%d,\"memory\":%d,\"status\":\"%s\"}",
            i ? "," : "", requests[i].id, requests[i].unit_id,
            requests[i].equipment_id, requests[i].quantity,
            requests[i].priority, requests[i].burst_time,
            requests[i].memory_required, status_name(requests[i].status));
    }
    snprintf(body + used, sizeof(body) - used, "]}");
    send_json(fd, 200, body);
}

static void get_files(int fd) {
    load_api_files();
    char body[12000];
    size_t used = 0;
    used += (size_t)snprintf(body + used, sizeof(body) - used, "{\"ok\":true,\"files\":[");
    for (int i = 0; i < api_file_count; ++i) {
        char n[128];
        json_escape(api_files[i].name, n, sizeof(n));
        used += (size_t)snprintf(body + used, sizeof(body) - used,
            "%s{\"id\":%d,\"name\":\"%s\",\"size\":%d,\"allocation\":\"Contiguous\"}",
            i ? "," : "", api_files[i].id, n, api_files[i].size);
    }
    snprintf(body + used, sizeof(body) - used, "]}");
    send_json(fd, 200, body);
}

static void get_dashboard(int fd) {
    int personnel = 0, total = 0, available = 0, pending = 0, completed = 0, cancelled = 0;
    int memory = 0;
    double burst = 0.0;

    for (int i = 0; i < unit_count; ++i) personnel += units[i].personnel_count;
    for (int i = 0; i < equipment_count; ++i) {
        total += equipment[i].quantity;
        available += equipment[i].available;
    }
    for (int i = 0; i < request_count; ++i) {
        if (requests[i].status == 0) pending++;
        else if (requests[i].status == 1) completed++;
        else cancelled++;
        memory += requests[i].memory_required;
        burst += requests[i].burst_time;
    }
    if (request_count) burst /= request_count;

    char body[4096];
    snprintf(body, sizeof(body),
        "{\"ok\":true,\"units\":%d,\"personnel\":%d,\"equipment\":%d,"
        "\"available\":%d,\"issued\":%d,\"pending\":%d,\"completed\":%d,"
        "\"cancelled\":%d,\"pendingMemory\":%d,\"averageBurst\":%.1f}",
        unit_count, personnel, total, available, total - available,
        pending, completed, cancelled, memory, burst);
    send_json(fd, 200, body);
}

static void get_report(int fd) {
    char body[4096];
    int personnel = 0, total = 0, available = 0, pending = 0, completed = 0;
    for (int i = 0; i < unit_count; ++i) personnel += units[i].personnel_count;
    for (int i = 0; i < equipment_count; ++i) {
        total += equipment[i].quantity;
        available += equipment[i].available;
    }
    for (int i = 0; i < request_count; ++i) {
        if (requests[i].status == 0) pending++;
        else if (requests[i].status == 1) completed++;
    }
    snprintf(body, sizeof(body),
        "{\"ok\":true,\"report\":\"MIL-LOG SYSTEM REPORT\\n==============================\\n\\n"
        "UNITS\\nTotal Units        : %d\\nTotal Personnel    : %d\\n\\n"
        "INVENTORY\\nEquipment Types    : %d\\nTotal Equipment    : %d\\nAvailable          : %d\\nIssued             : %d\\n\\n"
        "SUPPLY REQUESTS\\nTotal Requests     : %d\\nPending            : %d\\nCompleted          : %d\\n\\n"
        "OS MODULES\\nCPU Scheduling     : READY\\nMemory Allocation  : READY\\nPaging             : READY\\n"
        "Banker's Algorithm : READY\\nDeadlock Detection : READY\\nFile Allocation    : READY\"}",
        unit_count, personnel, equipment_count, total, available, total - available,
        request_count, pending, completed);
    send_json(fd, 200, body);
}

static void add_unit(int fd, const char *body) {
    if (unit_count >= MAX_UNITS) { send_error(fd, 400, "Maximum unit capacity reached."); return; }
    Unit u;
    memset(&u, 0, sizeof(u));
    if (!json_string(body, "name", u.name, sizeof(u.name)) ||
        !json_string(body, "location", u.location, sizeof(u.location)) ||
        !json_int(body, "personnel", &u.personnel_count) || u.personnel_count < 0) {
        send_error(fd, 400, "name, location and non-negative personnel are required.");
        return;
    }
    u.id = next_id_units();
    units[unit_count++] = u;
    saveUnits();
    char out[512];
    snprintf(out, sizeof(out), "{\"ok\":true,\"message\":\"Unit added\",\"id\":%d}", u.id);
    send_json(fd, 201, out);
}

static void add_equipment(int fd, const char *body) {
    if (equipment_count >= MAX_EQUIPMENT) { send_error(fd, 400, "Maximum equipment capacity reached."); return; }
    Equipment e;
    memset(&e, 0, sizeof(e));
    if (!json_string(body, "name", e.name, sizeof(e.name)) ||
        !json_string(body, "category", e.category, sizeof(e.category)) ||
        !json_int(body, "quantity", &e.quantity) || e.quantity < 0) {
        send_error(fd, 400, "name, category and non-negative quantity are required.");
        return;
    }
    e.id = next_id_equipment();
    e.available = e.quantity;
    equipment[equipment_count++] = e;
    saveInventory();
    char out[512];
    snprintf(out, sizeof(out), "{\"ok\":true,\"message\":\"Equipment added\",\"id\":%d}", e.id);
    send_json(fd, 201, out);
}

static void add_request(int fd, const char *body) {
    if (request_count >= MAX_REQUESTS) { send_error(fd, 400, "Maximum request capacity reached."); return; }
    SupplyRequest r;
    memset(&r, 0, sizeof(r));
    if (!json_int(body, "unit", &r.unit_id) ||
        !json_int(body, "equipment", &r.equipment_id) ||
        !json_int(body, "quantity", &r.quantity) ||
        !json_int(body, "priority", &r.priority) ||
        !json_int(body, "burst", &r.burst_time) ||
        !json_int(body, "memory", &r.memory_required)) {
        send_error(fd, 400, "unit, equipment, quantity, priority, burst and memory are required.");
        return;
    }
    if (r.quantity <= 0 || r.priority < 1 || r.priority > 3 ||
        r.burst_time <= 0 || r.memory_required <= 0) {
        send_error(fd, 400, "Invalid request values.");
        return;
    }

    int unit_ok = 0, ei = -1;
    for (int i = 0; i < unit_count; ++i) if (units[i].id == r.unit_id) unit_ok = 1;
    for (int i = 0; i < equipment_count; ++i)
        if (equipment[i].id == r.equipment_id) { ei = i; break; }

    if (!unit_ok) { send_error(fd, 400, "Invalid unit ID."); return; }
    if (ei < 0) { send_error(fd, 400, "Invalid equipment ID."); return; }
    if (r.quantity > equipment[ei].available) {
        send_error(fd, 400, "Requested quantity exceeds available inventory.");
        return;
    }

    r.id = next_id_request();
    r.status = 0;
    requests[request_count++] = r;
    saveRequests();

    char out[512];
    snprintf(out, sizeof(out), "{\"ok\":true,\"message\":\"Request created\",\"id\":%d}", r.id);
    send_json(fd, 201, out);
}

static void request_action(int fd, int id, int action) {
    for (int i = 0; i < request_count; ++i) {
        if (requests[i].id != id) continue;

        if (action == 0) {
            if (requests[i].status != 0) {
                send_error(fd, 400, "Only pending requests can be processed.");
                return;
            }
            int ei = -1;
            for (int j = 0; j < equipment_count; ++j)
                if (equipment[j].id == requests[i].equipment_id) { ei = j; break; }
            if (ei < 0) { send_error(fd, 400, "Equipment no longer exists."); return; }
            if (equipment[ei].available < requests[i].quantity) {
                send_error(fd, 400, "Insufficient equipment available.");
                return;
            }
            equipment[ei].available -= requests[i].quantity;
            requests[i].status = 1;
            saveInventory();
            saveRequests();
            send_json(fd, 200, "{\"ok\":true,\"message\":\"Request processed\"}");
            return;
        }

        if (requests[i].status == 1) {
            send_error(fd, 400, "Completed requests cannot be cancelled.");
            return;
        }
        if (requests[i].status == 2) {
            send_error(fd, 400, "Request is already cancelled.");
            return;
        }
        requests[i].status = 2;
        saveRequests();
        send_json(fd, 200, "{\"ok\":true,\"message\":\"Request cancelled\"}");
        return;
    }
    send_error(fd, 404, "Request not found.");
}

static void add_file(int fd, const char *body) {
    load_api_files();
    if (api_file_count >= 50) { send_error(fd, 400, "Maximum file capacity reached."); return; }

    ApiFileRecord f;
    memset(&f, 0, sizeof(f));
    if (!json_string(body, "name", f.name, sizeof(f.name)) ||
        !json_int(body, "size", &f.size) || f.size <= 0 || f.size > 100) {
        send_error(fd, 400, "name and size (1-100 blocks) are required.");
        return;
    }
    f.id = next_id_file();
    api_files[api_file_count++] = f;
    save_api_files();

    char out[512];
    snprintf(out, sizeof(out), "{\"ok\":true,\"message\":\"File created\",\"id\":%d}", f.id);
    send_json(fd, 201, out);
}

static void query_value(const char *query, const char *key, char *out, size_t n) {
    out[0] = '\0';
    if (!query) return;
    char needle[64];
    snprintf(needle, sizeof(needle), "%s=", key);
    const char *p = strstr(query, needle);
    if (!p) return;
    p += strlen(needle);
    size_t i = 0;
    while (*p && *p != '&' && i + 1 < n) out[i++] = *p++;
    out[i] = '\0';
}

static int hex_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static void url_decode(char *s) {
    char *src = s, *dst = s;
    while (*src) {
        if (*src == '%' && isxdigit((unsigned char)src[1]) &&
            isxdigit((unsigned char)src[2])) {
            int hi = hex_value(src[1]), lo = hex_value(src[2]);
            *dst++ = (char)((hi << 4) | lo);
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            ++src;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

static void decode_commas(char *s) {
    url_decode(s);
}

static int parse_list(const char *s, int *a, int max) {
    int n = 0;
    const char *p = s;
    while (*p && n < max) {
        char *end;
        long v = strtol(p, &end, 10);
        if (end == p) break;
        a[n++] = (int)v;
        p = end;
        if (*p == ',') ++p;
        else break;
    }
    return n;
}

static void url_decode(char *s);

static void os_memory(int fd, const char *query) {
    char method[32], bs[1024], ps[1024];
    query_value(query, "method", method, sizeof(method));
    url_decode(method);
    query_value(query, "blocks", bs, sizeof(bs));
    query_value(query, "processes", ps, sizeof(ps));
    decode_commas(bs); decode_commas(ps);

    int b[100], p[100], nblocks = parse_list(bs, b, 100), nproc = parse_list(ps, p, 100);
    int used[100] = {0}, allocation[100];
    for (int i = 0; i < nproc; ++i) allocation[i] = -1;

    for (int i = 0; i < nproc; ++i) {
        int chosen = -1;
        for (int j = 0; j < nblocks; ++j) {
            if (used[j] || b[j] < p[i]) continue;
            if (chosen < 0 ||
                (strcmp(method, "Best Fit") == 0 && b[j] < b[chosen]) ||
                (strcmp(method, "Worst Fit") == 0 && b[j] > b[chosen]) ||
                (strcmp(method, "First Fit") == 0 && j < chosen)) chosen = j;
        }
        if (chosen >= 0) { allocation[i] = chosen; used[chosen] = 1; }
    }

    char body[10000]; size_t u = 0;
    u += (size_t)snprintf(body+u, sizeof(body)-u, "{\"ok\":true,\"method\":\"%s\",\"results\":[", method);
    for (int i = 0; i < nproc; ++i) {
        u += (size_t)snprintf(body+u, sizeof(body)-u, "%s{\"process\":%d,\"block\":%d}",
            i ? "," : "", p[i], allocation[i] < 0 ? 0 : allocation[i] + 1);
    }
    snprintf(body+u, sizeof(body)-u, "]}");
    send_json(fd, 200, body);
}

static void os_paging(int fd, const char *query) {
    char algo[32], rs[2048], fs[32];
    query_value(query, "algorithm", algo, sizeof(algo));
    url_decode(algo);
    query_value(query, "refs", rs, sizeof(rs));
    query_value(query, "frames", fs, sizeof(fs));
    decode_commas(rs);
    int refs[200], n = parse_list(rs, refs, 200);
    int frames = atoi(fs); if (frames < 1) frames = 3; if (frames > 20) frames = 20;
    int f[20], last[20], hits = 0, faults = 0;
    for (int i=0;i<frames;i++){f[i]=-1;last[i]=-1;}
    int pointer=0;
    for(int i=0;i<n;i++){
        int pos=-1;
        for(int j=0;j<frames;j++) if(f[j]==refs[i]){pos=j;break;}
        if(pos>=0){hits++;last[pos]=i;continue;}
        faults++;
        int repl=-1;
        for(int j=0;j<frames;j++) if(f[j]==-1){repl=j;break;}
        if(repl<0 && strcmp(algo,"FIFO")==0){repl=pointer;pointer=(pointer+1)%frames;}
        else if(repl<0 && strcmp(algo,"LRU")==0){
            repl=0; for(int j=1;j<frames;j++) if(last[j]<last[repl]) repl=j;
        } else if(repl<0){
            int farthest=-1;
            for(int j=0;j<frames;j++){
                int k; for(k=i+1;k<n;k++) if(f[j]==refs[k]) break;
                if(k==n){repl=j;break;}
                if(k>farthest){farthest=k;repl=j;}
            }
        }
        f[repl]=refs[i]; last[repl]=i;
    }
    char body[1024];
    snprintf(body,sizeof(body),"{\"ok\":true,\"algorithm\":\"%s\",\"frames\":%d,\"hits\":%d,\"faults\":%d,\"total\":%d}",algo,frames,hits,faults,n);
    send_json(fd,200,body);
}

static void os_scheduling(int fd, const char *query) {
    char algo[32], qs[32];
    query_value(query,"algorithm",algo,sizeof(algo));
    url_decode(algo);
    query_value(query,"quantum",qs,sizeof(qs));
    int quantum=atoi(qs); if(quantum<1) quantum=2;

    int idx[200], n=0;
    for(int i=0;i<request_count;i++) if(requests[i].status==0) idx[n++]=i;
    if(n==0){send_json(fd,200,"{\"ok\":true,\"algorithm\":\"No pending requests\",\"segments\":[],\"total\":0}");return;}

    if(strcmp(algo,"SJF")==0){
        for(int i=0;i<n-1;i++) for(int j=i+1;j<n;j++)
            if(requests[idx[j]].burst_time < requests[idx[i]].burst_time){int t=idx[i];idx[i]=idx[j];idx[j]=t;}
    } else if(strcmp(algo,"Priority")==0){
        for(int i=0;i<n-1;i++) for(int j=i+1;j<n;j++)
            if(requests[idx[j]].priority < requests[idx[i]].priority){int t=idx[i];idx[i]=idx[j];idx[j]=t;}
    }

    char body[16000]; size_t u=0;
    u+=(size_t)snprintf(body+u,sizeof(body)-u,"{\"ok\":true,\"algorithm\":\"%s\",\"segments\":[",algo);
    int time=0, first=1;
    if(strcmp(algo,"Round Robin")==0){
        int rem[200]; for(int i=0;i<n;i++) rem[i]=requests[idx[i]].burst_time;
        int done=0;
        while(done<n){
            for(int i=0;i<n;i++) if(rem[i]>0){
                int z=rem[i]>quantum?quantum:rem[i];
                u+=(size_t)snprintf(body+u,sizeof(body)-u,"%s{\"id\":%d,\"start\":%d,\"end\":%d}",first?"":",",requests[idx[i]].id,time,time+z);
                first=0; time+=z; rem[i]-=z; if(rem[i]==0) done++;
            }
        }
    } else {
        for(int i=0;i<n;i++){
            int z=requests[idx[i]].burst_time;
            u+=(size_t)snprintf(body+u,sizeof(body)-u,"%s{\"id\":%d,\"start\":%d,\"end\":%d}",first?"":",",requests[idx[i]].id,time,time+z);
            first=0; time+=z;
        }
    }
    snprintf(body+u,sizeof(body)-u,"],\"total\":%d}",time);
    send_json(fd,200,body);
}

static void handle_request(int fd, char *req, int req_len) {
    (void)req_len;
    char method[16]={0}, path[2048]={0};
    if (sscanf(req, "%15s %2047s", method, path) != 2) {
        send_error(fd,400,"Invalid HTTP request.");
        return;
    }

    if (strcmp(method,"OPTIONS")==0) {
        send_json(fd,200,"{\"ok\":true}");
        return;
    }

    char *body = strstr(req,"\r\n\r\n");
    body = body ? body + 4 : (char*)"";
    char *query = strchr(path,'?');
    if (query) { *query='\0'; ++query; }

    if (strcmp(method,"GET")==0) {
        if(strcmp(path,"/api/dashboard")==0) get_dashboard(fd);
        else if(strcmp(path,"/api/units")==0) get_units(fd);
        else if(strcmp(path,"/api/inventory")==0) get_inventory(fd);
        else if(strcmp(path,"/api/requests")==0) get_requests(fd);
        else if(strcmp(path,"/api/files")==0) get_files(fd);
        else if(strcmp(path,"/api/reports")==0) get_report(fd);
        else if(strcmp(path,"/api/scheduling")==0) os_scheduling(fd,query);
        else if(strcmp(path,"/api/memory")==0) os_memory(fd,query);
        else if(strcmp(path,"/api/paging")==0) os_paging(fd,query);
        else if(strcmp(path,"/api/health")==0) send_json(fd,200,"{\"ok\":true,\"service\":\"MIL-LOG API\",\"port\":8090}");
        else send_error(fd,404,"API endpoint not found.");
        return;
    }

    if(strcmp(method,"POST")==0) {
        if(strcmp(path,"/api/units")==0) add_unit(fd,body);
        else if(strcmp(path,"/api/inventory")==0) add_equipment(fd,body);
        else if(strcmp(path,"/api/requests")==0) add_request(fd,body);
        else if(strcmp(path,"/api/files")==0) add_file(fd,body);
        else {
            int id;
            if(sscanf(path,"/api/requests/%d/process",&id)==1 && strstr(path,"/process")) request_action(fd,id,0);
            else if(sscanf(path,"/api/requests/%d/cancel",&id)==1 && strstr(path,"/cancel")) request_action(fd,id,1);
            else send_error(fd,404,"API endpoint not found.");
        }
        return;
    }

    send_error(fd,400,"Only GET, POST and OPTIONS are supported.");
}

int main(void) {
    signal(SIGPIPE, SIG_IGN);

    loadUnits();
    loadInventory();
    loadRequests();
    load_api_files();

    int server = socket(AF_INET, SOCK_STREAM, 0);
    if(server < 0){perror("socket");return 1;}

    int yes=1;
    setsockopt(server,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));

    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    addr.sin_port=htons(PORT);

    if(bind(server,(struct sockaddr*)&addr,sizeof(addr))<0){perror("bind");close(server);return 1;}
    if(listen(server,16)<0){perror("listen");close(server);return 1;}

    printf("MIL-LOG API server running at http://127.0.0.1:%d\n",PORT);
    printf("Press Ctrl+C to stop.\n");

    while(1){
        int client=accept(server,NULL,NULL);
        if(client<0){if(errno==EINTR)continue;perror("accept");break;}

        char *req=calloc(1,BUFFER_SIZE);
        if(!req){close(client);continue;}
        int total=0;
        int header_end=0;
        while(total<BUFFER_SIZE-1){
            int n=(int)recv(client,req+total,(size_t)(BUFFER_SIZE-1-total),0);
            if(n<=0)break;
            total+=n;
            req[total]='\0';
            char *marker=strstr(req,"\r\n\r\n");
            if(marker){
                header_end=(int)(marker-req)+4;
                break;
            }
        }

        if(header_end > 0){
            int content_length=0;
            char *cl=strstr(req,"Content-Length:");
            if(!cl) cl=strstr(req,"content-length:");
            if(cl) content_length=atoi(cl+15);
            int body_have=total-header_end;
            while(body_have<content_length && total<BUFFER_SIZE-1){
                int want=content_length-body_have;
                if(want>BUFFER_SIZE-1-total) want=BUFFER_SIZE-1-total;
                int n=(int)recv(client,req+total,(size_t)want,0);
                if(n<=0)break;
                total+=n;
                body_have+=n;
                req[total]='\0';
            }
        }
        handle_request(client,req,total);
        free(req);
        close(client);
    }

    close(server);
    return 0;
}
