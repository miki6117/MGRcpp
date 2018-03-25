module dataGenerator(
    input wire [31:0] pattern,
    input wire        clk,
    input wire        enable_gener,
    input wire        reset,

    output reg [63:0] dataout,
    output reg        dataout_available
);

always @(posedge clk) begin
    if (reset) begin
        dataout_available = 0;
        case (pattern)
            3'b000: begin
                dataout[7:0] = -8'h8;;
                dataout[15:8] = -8'h7;
                dataout[23:16] = -8'h6;
                dataout[31:24] = -8'h5; 
                dataout[39:32] = -8'h4; 
                dataout[47:40] = -8'h3; 
                dataout[55:48] = -8'h2; 
                dataout[63:56] = -8'h1; 
            end
            3'b001: dataout = -64'b1;
            3'b010: dataout = 64'b1000000000000000000000000000000000000000000000000000000000000000;
        endcase
    end

    if (enable_gener) begin
        dataout_available = 1;
        case (pattern)
            3'b000: begin
                dataout[7:0] = dataout[7:0] + 4'b1000;
                dataout[15:8] = dataout[15:8] + 4'b1000;
                dataout[23:16] = dataout[23:16] + 4'b1000;
                dataout[31:24] = dataout[31:24] + 4'b1000;
                dataout[39:32] = dataout[39:32] + 4'b1000;
                dataout[47:40] = dataout[47:40] + 4'b1000;
                dataout[55:48] = dataout[55:48] + 4'b1000;
                dataout[63:56] = dataout[63:56] + 4'b1000;
            end
            3'b001: dataout = dataout + 64'b1;
            3'b010: dataout = {dataout[62:0], dataout[63]};
        endcase
    end else begin
        dataout_available = 0;
    end
end

endmodule
