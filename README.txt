MIL-LOG v1.0
Military Logistics Management System using OS Resource Management Techniques

FINAL PROJECT STRUCTURE
-----------------------
backend/   Finalized C backend + C HTTP/JSON API bridge
frontend/  HTML/CSS/JavaScript logistics interface
 data/     Persistent binary .dat storage

CORE BACKEND (UNCHANGED)
-------------------------
1. Unit Management
2. Equipment & Inventory
3. Supply Requests
4. Process Scheduling
5. Memory Management
6. Virtual Memory / Paging
7. Resource Allocation & Deadlock
8. File Management
9. Reports

FRONTEND MAPPING
----------------
Supply Mission Scheduling -> Supply requests are treated as processes.
Depot Storage Allocation -> Supply loads are treated as memory requests.
Mission Data Paging      -> Mission/logistics records are treated as pages.
Equipment Resource Safety-> Units/processes share scarce military resources.
Military Record Management-> Logistics records use file management concepts.

RUN API SERVER (WSL)
--------------------
cd /mnt/c/Users/sp786/Desktop/MIL-LOG/backend
./mil-log-api

RUN FRONTEND (second WSL terminal)
----------------------------------
cd /mnt/c/Users/sp786/Desktop/MIL-LOG/frontend
python3 -m http.server 5500

OPEN IN BROWSER
---------------
http://127.0.0.1:5500

API HEALTH CHECK
----------------
http://127.0.0.1:8090/api/health

NOTE
----
The core OS/backend implementation remains separate and intact. The API is an integration layer that exposes selected finalized backend data and simulations to the browser.
