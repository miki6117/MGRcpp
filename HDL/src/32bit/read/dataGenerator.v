module dataGenerator(
    input wire [31:0] pattern,
    input wire        clk,
    input wire        enable_gener,
    input wire        reset,

    output reg [31:0] dataout,
    output reg        dataout_available
);

always @(posedge clk) begin
    if (reset) begin
        dataout_available = 0;
        case (pattern)
            3'b000: begin
                dataout[7:0] = -8'h4;;
                dataout[15:8] = -8'h3;
                dataout[23:16] = -8'h2;
                dataout[31:24] = -8'h1; 
            end
            3'b001: dataout = -32'b1;
            3'b010: dataout = 32'b10000000000000000000000000000000;
        endcase
    end

    if (enable_gener) begin
        dataout_available = 1;
        case (pattern)
            3'b000: begin
                dataout[7:0] = dataout[7:0] + 3'b100;
                dataout[15:8] = dataout[15:8] + 3'b100;
                dataout[23:16] = dataout[23:16] + 3'b100;
                dataout[31:24] = dataout[31:24] + 3'b100;
            end
            3'b001: dataout = dataout + 32'b1;
            3'b010: dataout = {dataout[30:0], dataout[31]};
        endcase
    end else begin
        dataout_available = 0;
    end
end

endmodule
