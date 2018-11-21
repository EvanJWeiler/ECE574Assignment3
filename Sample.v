`timescale 1 ns/1 ns
module TimeVerifier(Clk, Rst, CStart, CEnd, ErrorRst, Error);
   input Clk, Rst;
   input CStart, CEnd, ErrorRst;
   output reg Error;
   
   //Variables need to be added from allVariables - was there for the last project...
   input [15:0] a, b, c, d;
   output reg dLTe;
   output reg [15:0] e, f;
   
   parameter S_Wait = 0, 
             S_Cycle1 = 1, 
             S_Cycle2 = 2, 
             S_Cycle3 = 3
             S_CycleEnd = 4, 
             S_Error = 5;
             
   reg [2:0] State, StateNext;
   
   // Comb logic for outputs and next state transitions that is constant
   //always @(State, CStart, CEnd, ErrorRst) begin
      //FOR IF statements we must sense for the variable ex: if(dLTe)
   always @(State, CStart, CEnd, ErrorRst, dLTe) begin
         
      case (State) 
         S_Wait : begin
            Error <= 0;
            if (CStart == 1) begin
               StateNext <= S_Cycle1;
            end 
            else begin
               StateNext <= S_Wait;
            end
         end
         
         S_Cycle1 : begin
            Error <= 0;
            //Operations will have to be added based on scheduled time
            e = a + b;
            dLTe = a < b;
            if (CEnd == 1) begin
               StateNext <= S_CycleEnd;
            end
            else begin 
               StateNext <= S_Cycle2;
            end
         end
         
         S_Cycle2 : begin
            Error <= 0;
            //Operations will have to be added based on scheduled time  WITH IF STATEMENT
            f = dLTe ? c : d;
            if (dLTe) begin
               StateNext <= S_Cycle3;
            end
            else if (CEnd == 1) begin
               StateNext <= S_CycleEnd;
            end
            else begin
               StateNext <= S_Error;
            end
         end
         
         S_Cycle3 : begin
            Error <= 0;
            e <= b + c;
            if (CEnd == 1)
               StateNext <= S_CycleEnd;
            else 
               StateNext <= S_Error;            
         end
         
         S_CycleEnd : begin
            Error <= 0;
            if (CEnd == 1) begin
               StateNext <= S_Error;
            end
            else begin
               StateNext <= S_Wait;
            end
         end
         
         S_Error : begin
             Error <= 1;
             if (ErrorRst == 1) begin
                StateNext <= S_Wait;
             end
             else begin
                StateNext <= S_Error;
             end
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






