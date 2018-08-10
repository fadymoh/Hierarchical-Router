# Hierarchical Router
This project was a group project done by me, Hassan Osama Kandil, Omar ElSeadawy, and Ahmed Elrouby.
This is a hierarchical router that uses A* search algorithm to route given pairs of nets.

The routing starts by parsing the given DEF & LEF files using unordered_maps (Hash Tables), to map the gates and their pin locations to the given die and divide the grid using the tracks found inside the DEF file.

The grid is created by getting the GCD of all the X tracks and the GCD of all the Y tracks to make grid layers of common size, and then 
the GCD is used to block a certain number of tracks according to the ratio between the GCD and the given track's pitch.

Next, ordering the nets is based on finding the length of the net. A rectilinear Minimum Steiner Tree is used to calculate the length
of net and then pushed on a priority queue according to the length along with the net name.

Finally, in the routing phase, a global grid is created using the given GBOXsize, and A* is used to route the given pins globally and then
detailed after constructing a tempPath from the directions returned from the global routing. The Global routing function uses 2 functions from the parser class to get the net names and then using that net name get a vector of pairs of triplets (data structure to encapsulate the 3 coordinates of any pin x, y, z) containing all the routing coordinates of that net. And then the output is printed in the DEF file.

The program is able to route 6000 nets in 10 seconds and prints the output routed DEF that can be viewed using any layout viewer.

It is an open source, so anyone can take the code and optimize, improve, and add on it. Just give a star :)

Current limitations:
1) Rip & Reroute has to be implemented.
2) Need to take all the rects of each pin into account, as when a pin fails to route from one rect, the other has to                           be taken into account.
3) The obstacles of all the cells has to be taken into account.
4) Making the code more Object-Oriented.
5) Collapse the layers on each other.
                     
Build Commands: g++ Source.cpp Parser.cpp neighbors.cpp mst2.cpp memAlloc.cpp heap.cpp flute_mst.cpp flute.cpp err.cpp dl.cpp dist.cpp bookshelf_IO.cpp -std=c++11
Command line args: Def_file Lef_file Gbox Congestion enable_output
Example:           cpu_unroute.def  osu035.lef  10  40  1

If you have any comments, feel free to message me at fadyabuelmagd@gmail.com
