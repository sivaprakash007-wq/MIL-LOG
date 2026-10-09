MIL-LOG v1.0 - FINAL FULL-STACK PROJECT
========================================

Architecture
------------
Frontend: HTML + CSS + JavaScript
API/server: C HTTP server
Backend: Existing finalized C OS + logistics implementation
Storage: Binary .dat files under ../data/
Communication: HTTP + JSON

IMPORTANT
---------
The original C backend modules were not rewritten. The frontend now presents the existing OS modules as logistics operations:

Supply Mission Scheduling -> supply requests are processes
Depot Storage Allocation -> supply loads are memory requests
Mission Data Paging -> mission records are pages
Equipment Resource Safety -> units/processes use shared equipment resources
Military Record Management -> logistics records use file management concepts

Run
---
Terminal 1:
  cd /mnt/c/Users/sp786/Desktop/MIL-LOG/backend
  ./mil-log-api

Terminal 2:
  cd /mnt/c/Users/sp786/Desktop/MIL-LOG/frontend
  python3 -m http.server 5500

Open:
  http://127.0.0.1:5500

API:
  http://127.0.0.1:8090/api/health

The C API reads and writes the existing ../data/*.dat files.
The core backend remains available separately as ./mil-log.
