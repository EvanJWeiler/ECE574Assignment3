//input Int32 a, b, c
//output Int32 z, x
//variable Int32 d, f, g, zrin
//d = a + b
//g = a < b
//zrin = a + c
//if ( g ) {
//	zrin = a + b
//}
//f = a * c
//x = f - d  
//z = zrin + f
//Below is the output of the above 'if'


`timescale 1ns / 1ps
module TimeVerifier(Clk, Rst, CStart, CEnd, ErrorRst, Error, a, b, c, z, x);
   input Clk, Rst;
   input CStart, CEnd, ErrorRst;
   output Error;
   
   input [31:0] a, b, c;
   output reg [31:0] z, x;
   reg [31:0] d, f, g, zin;
   
   parameter S_CycleEnd = 0,
            State1 = 1,
            State2 = 2,  //if
            State3 = 3,
            State4 = 4,
            State5 = 5,
            S_Wait = 6;
  reg [2:0] State, NextState;
  
  always@(CStart, CEnd, ErrorRst, Error) begin
        case(State)
            S_Wait: begin
   	            if(CStart == 1)
	                  StateNext <= State1;
	              else
	                  StateNext <= S_Wait;
	          end
	          State1: begin
	                d = a + b;
                  g = a < b;
                  zrin = a + c;
                  if(g)
                    StateNext <= State2;
                  else
                    StateNext <= State3;
            State2: begin
   	              zrin = a + b
                  StateNext <= State3;
	          end
	          State3: begin
                  f = a * c;
                  StateNext <= State4;
            end
            State4: begin
                  x = f - d;
                  StateNext <= State5;
            end
            State5: begin
                  z = zrin + f;
                  StateNext <= S_CycleEnd;
            end
            S_CycleEnd: begin
                  if(Rst == 1)
	                    StateNext <= S_Wait;
	                else
	                    StateNext <= S_CycleEnd;
	          end
	  endcase
 end
 always @(posedge Clk) begin
	if (Rst == 1) begin
	   State <= S_Wait;
	end
