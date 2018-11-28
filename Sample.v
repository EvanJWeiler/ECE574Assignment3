`timescale 1 ns/1 ns
module TimeVerifier(Clk, Rst, CStart, CEnd, ErrorRst, Error, a, b, c, z, x);
   input Clk, Rst;
   input CStart, CEnd, ErrorRst;
   output reg Error;
   
   //Variables need to be added from allVariables - was there for the last project...
   input [31:0] a, b, c;
   output reg [31:0] z, x;
   variable [31:0] d, e, f, g, h;
   variable unsigned dLTe, dEQe;
   
   parameter S_Wait = 0, 
             State1 = 1, 
             State2 = 2, 
             State3 = 3
             State4 = 4, 
             State5 = 5 
             S_CycleEnd = 6;
             
   reg [2:0] State, StateNext;
   
   // Comb logic for outputs and next state transitions that is constant
   always @(State, CStart, CEnd, ErrorRst) begin
      //FOR IF statements we must sense for the variable ex: if(dLTe)
   //always @(State, CStart, CEnd, ErrorRst, dLTe) begin
         
      case (State) begin
         Wait: begin
            if(CStart == 1)
               NextState <= State1;
            else
               NextState <= Wait;
         end
         State1: begin
            d <= a + b;		//Based on schedule time of 1
            e <= a + c;		//Based on schedule time of 1
            f <= a - b;		//Based on schedule time of 1
            NextState <= State2;
         end
         State2: begin
            dEQe <= d == e;		//Based on schedule time of 2
            dLTe <= d < e;		//Based on schedule time of 2
            NextState <= State3;
         end
         State3: begin
            g <= dLTe ? d : e;	//Based on schedule time of 3
            NextState <= State4;
         end
         State4: begin
            h <= dEQe ? g : f;	//Based on schedule time of 4
            x <= g << dLTe;		//Based on schedule time of 4
            NextState <= State5;
         end
         State5: begin
            z <= h >> dEQe;		//Based on schedule time of 5
            NextState <= S_CycleEnd;
         end
         S_CycleEnd: begin
            if(Rst == 1)
               NextState <= Wait;
            else
               NextState <= S_CycleEnd;
         end
      endcase
   end
   
   // State regsiter
   always @(posedge Clk) begin
      if (Rst == 1) begin
         State <= S_Wait;
      end
      else begin
         State <= StateNext;
      end 
   end
endmodule






