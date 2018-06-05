# Hierarchical Router
This project was a group project done by me, Hassan Osama Kandil, Omar ElSeadawy, and Ahmed Elrouby.
This is a hierarchical router that uses A* search algorithm to route given pairs of nets.

The routing starts by parsing the given DEF & LEF files using unordered_maps (Hash Tables), to map the gates and their pin locations to the given die and divide the grid using the tracks found inside the DEF file.

The program is able to route 6000 nets in 5 mins and prints the output routed DEF that can be viewed using any layout viewer.

It is an open source, so anyone can take the code and optimize, improve, and add on it. Just give a star :)

Current limitations: 1) Rip & Reroute has to be implemented.
                     2) Need to take all the rects of each pin into account, as when a pin fails to route from one rect, the other has to                           be taken into account.
                     3) The obstacles of all the cells has to be taken into account.
                     4) Making the code more Object-Oriented.
                     5) Collapse the layers on each other.
                     
Build Commands: g++ Source.cpp Parser.cpp -std=c++11
Command line args: Def_file Lef_file Gbox Congestion enable_output
Example:           bcd.def  osu035.lef  5   50  1

If you have any comments, feel free to message me at fadyabuelmagd@gmail.com
