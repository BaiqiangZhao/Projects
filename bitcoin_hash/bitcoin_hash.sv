module bitcoin_hash (input logic        clk, reset_n, start,
                     input logic [15:0] message_addr, output_addr,
                    output logic        done, mem_clk, mem_we,
                    output logic [15:0] mem_addr,
                    output logic [31:0] mem_write_data,
                     input logic [31:0] mem_read_data);

parameter num_nonces = 16;

//logic [ 4:0] state;
logic [31:0] hout[num_nonces];

parameter int k[64] = '{
    32'h428a2f98,32'h71374491,32'hb5c0fbcf,32'he9b5dba5,32'h3956c25b,32'h59f111f1,32'h923f82a4,32'hab1c5ed5,
    32'hd807aa98,32'h12835b01,32'h243185be,32'h550c7dc3,32'h72be5d74,32'h80deb1fe,32'h9bdc06a7,32'hc19bf174,
    32'he49b69c1,32'hefbe4786,32'h0fc19dc6,32'h240ca1cc,32'h2de92c6f,32'h4a7484aa,32'h5cb0a9dc,32'h76f988da,
    32'h983e5152,32'ha831c66d,32'hb00327c8,32'hbf597fc7,32'hc6e00bf3,32'hd5a79147,32'h06ca6351,32'h14292967,
    32'h27b70a85,32'h2e1b2138,32'h4d2c6dfc,32'h53380d13,32'h650a7354,32'h766a0abb,32'h81c2c92e,32'h92722c85,
    32'ha2bfe8a1,32'ha81a664b,32'hc24b8b70,32'hc76c51a3,32'hd192e819,32'hd6990624,32'hf40e3585,32'h106aa070,
    32'h19a4c116,32'h1e376c08,32'h2748774c,32'h34b0bcb5,32'h391c0cb3,32'h4ed8aa4a,32'h5b9cca4f,32'h682e6ff3,
    32'h748f82ee,32'h78a5636f,32'h84c87814,32'h8cc70208,32'h90befffa,32'ha4506ceb,32'hbef9a3f7,32'hc67178f2
};

// Student to add rest of the code here
logic   [31:0] h0[num_nonces];
logic   [31:0] h1[num_nonces];
logic   [31:0] h2[num_nonces];
logic   [31:0] h3[num_nonces];
logic   [31:0] h4[num_nonces];
logic   [31:0] h5[num_nonces];
logic   [31:0] h6[num_nonces];
logic   [31:0] h7[num_nonces];
logic [31:0] hash[16]; 
logic [15:0] cur_addr;
logic [31:0] cur_write_data;
logic        cur_we, first_block, second_block, third_phase ;
logic [15:0] offset;
logic [31:0] message[20];
logic [31:0] fh0, fh1, fh2, fh3, fh4, fh5, fh6, fh7;
logic [31:0] a, b, c, d, e, f, g, h;
logic [7:0] i;
logic [31:0] w[16];
logic [31:0] n;

assign mem_clk = clk;
assign mem_addr = cur_addr + offset;
assign mem_we = cur_we;
assign mem_write_data = cur_write_data;

parameter NUM_WORDS = 20;
enum logic [2:0] {IDLE, READ, EMPTY, BLOCK_1, BLOCK_2, PHASE_3, COMPUTE, WRITE} state;


function logic [31:0] rightrotate(input logic [31:0] x,
                                  input logic [ 7:0] r);
   // Student to add function implementation
	rightrotate = (x >> r) | (x << (32 - r)); 


endfunction


function logic [31:0] wtnew;
	logic [31:0] s0,s1;
	s0 = rightrotate(w[1],7) ^ rightrotate(w[1],18) ^ (w[1] >> 3);
	s1 = rightrotate(w[14],17) ^ rightrotate(w[14],19) ^ (w[14] >> 10);
	wtnew = w[0] + s0 + w[9] + s1;
endfunction

// SHA256 hash round
function logic [255:0] sha256_op(input logic [31:0] a, b, c, d, e, f, g, h, w, k);
	logic [31:0] S1, S0, ch, maj, t1, t2; // internal signals
begin
   S1 = rightrotate(e, 6) ^ rightrotate(e, 11) ^ rightrotate(e, 25);
	ch = (e & f) ^ ((~e) & g);
	t1 = ch + S1 + h + k + w;
	S0 = rightrotate(a, 2) ^ rightrotate(a, 13) ^ rightrotate(a, 22);
	maj = (a & b) ^ (a & c) ^ (b & c);
	t2 = maj + S0;
	sha256_op = { t1 + t2 , a, b, c, d + t1 , e, f,g};
	end
endfunction


always@(posedge clk, negedge reset_n) begin
	if(!reset_n) begin
		done <= 0;
		state <= IDLE;
	end
	else 
	
	case(state)
		IDLE: begin
			if(start) begin
				
				state <= IDLE;
				
				fh0 <= 32'h6a09e667;
				fh1 <= 32'hbb67ae85;
				fh2 <= 32'h3c6ef372;
				fh3 <= 32'ha54ff53a;
				fh4 <= 32'h510e527f;
				fh5 <= 32'h9b05688c;
				fh6 <= 32'h1f83d9ab;
				fh7 <= 32'h5be0cd19;

				a <= 32'h6a09e667;
				b <= 32'hbb67ae85;
				c <= 32'h3c6ef372;
				d <= 32'ha54ff53a;
				e <= 32'h510e527f;
				f <= 32'h9b05688c;
				g <= 32'h1f83d9ab;
				h <= 32'h5be0cd19;		
		
				cur_we <= 0;
				offset <= 0;
				n <= 0;
				i <= 0;
				first_block <= 1;
				second_block <= 0;
				third_phase <= 0;
				state <= EMPTY;
				cur_addr <= message_addr;
				done <= 0;
				
			end
		end
	
		READ: begin
			if(offset < NUM_WORDS) begin 
				message[offset] <= mem_read_data;
				offset <= offset + 1;
				state <= EMPTY;
			end
			else begin 
				state <= BLOCK_1;
				offset <= 0;
			end
		end
		BLOCK_1: begin
			if (i < 16) begin
				w[i] <= message[i];
				i <= i + 1;
				state <= BLOCK_1;
				end
			else begin
				state <= COMPUTE;
				i <= 0;
			end
		end
		
		EMPTY: begin
			state <= READ;
		end
		
		BLOCK_2: begin
			if (n < num_nonces) begin 
		
				w[0] <= message[16];
				w[1] <= message[17];
				w[2] <= message[18];
				w[3] <= n;
				w[4] <= 32'h80000000;

				for (int i = 5; i < 15; i++) begin
					w[i] <= 32'h00000000;
				end
				w[15] <= 32'd640;
			
				state <= COMPUTE;
			
				h0[n] <= fh0;
				h1[n] <= fh1;
				h2[n] <= fh2;
				h3[n] <= fh3;
				h4[n] <= fh4;
				h5[n] <= fh5;
				h6[n] <= fh6;
				h7[n] <= fh7;
				
				a <= fh0;
				b <= fh1;
				c <= fh2;
				d <= fh3;
				e <= fh4;
				f <= fh5;
				g <= fh6;
				h <= fh7;
			end
			else begin
				state <= WRITE;
				n <= 0;
			end
			
		end
		
		PHASE_3: begin
			if (n < num_nonces) begin
				w[0] <= h0[n];
				w[1] <= h1[n];
				w[2] <= h2[n];
				w[3] <= h3[n];
				w[4] <= h4[n];
				w[5] <= h5[n];
				w[6] <= h6[n];
				w[7] <= h7[n];	
				w[8] <= 32'h80000000;
				for (int i = 9; i < 15; i++) begin
					w[i] <= 32'h00000000;
				end
				w[15] <= 32'd256; // SIZE = 256 BITS
			
				state <= COMPUTE;
				
				// INITIAL HASH

				h0[n] = 32'h6a09e667;
				h1[n] = 32'hbb67ae85;
				h2[n] = 32'h3c6ef372;
				h3[n] = 32'ha54ff53a;
				h4[n] = 32'h510e527f;
				h5[n] = 32'h9b05688c;
				h6[n] = 32'h1f83d9ab;
				h7[n] = 32'h5be0cd19;

				a = 32'h6a09e667;
				b = 32'hbb67ae85;
				c = 32'h3c6ef372;
				d = 32'ha54ff53a;
				e = 32'h510e527f;	
				f = 32'h9b05688c;
				g = 32'h1f83d9ab;
				h = 32'h5be0cd19;
				
			end
			else begin
				state <= WRITE;
				n <= 0;
			end
			
		end
		
		COMPUTE: begin
	// 64 processing rounds steps for 512-bit block 
	
			for (int n = 0; n < 15; n++) begin
				w[n] <= w[n+1];
			
			end
			w[15] <= wtnew();
		
		
		
			if (i < 64) begin
			
				{a, b, c, d, e, f, g, h} <= sha256_op(a, b, c, d, e, f, g, h, w[0],k[i]);
			
				i <= i+1;
				state <= COMPUTE;
			end
			else begin
				i <= 0;
				if (first_block == 1) begin  
					fh0 <= fh0 + a;
					fh1 <= fh1 + b;
					fh2 <= fh2 + c;
					fh3 <= fh3 + d;
					fh4 <= fh4 + e;
					fh5 <= fh5 + f;
					fh6 <= fh6 + g;
					fh7 <= fh7 + h;
				
					state <= BLOCK_2;
					first_block <= 0;
					second_block <= 1;
					
				end
				else begin
					h0[n] <= h0[n] + a;
					h1[n] <= h1[n] + b;
					h2[n] <= h2[n] + c;
					h3[n] <= h3[n] + d;
					h4[n] <= h4[n] + e;
					h5[n] <= h5[n] + f;
					h6[n] <= h6[n] + g;
					h7[n] <= h7[n] + h;
					
					if (second_block == 1) begin 
						state <= PHASE_3;
						
						third_phase <= 1;
						second_block <= 0;
						
					end
					if (third_phase == 1) begin
						state <= BLOCK_2;
						
						third_phase <= 0;
						second_block <= 1;
						
						n <= n + 1;
					end
					
			
				end
			end
		end
	 		
		WRITE: begin
			cur_we <= 1;
			cur_addr <= output_addr - 1;
			
			if (offset < 16) begin
				cur_write_data <= h0[offset];
				offset <= offset + 1;
				state <= WRITE;
			end
			else begin
				state <= IDLE;
				done <= 1;			
			end
				
			/*
			if (offset == 0) begin
				cur_write_data <= h0[0];
				offset <= offset + 1;
				state <= WRITE;
			end
			else if (offset == 1) begin		
				cur_write_data <= h0[1] ;
				offset <= offset + 1;
				state <= WRITE;
			end
			else if (offset == 2) begin			
				cur_write_data <= h0[2] ;
				offset <= offset + 1;
				state <= WRITE;
			end
			else if (offset == 3) begin
			
			
				cur_write_data <= h0[3] ;
				offset <= offset + 1;
				state <= WRITE;
			end
			else if (offset == 4) begin
			
			
				cur_write_data <= h0[4] ;
				offset <= offset + 1;
				state <= WRITE;
			end
			else if (offset == 5) begin
			
				
				cur_write_data <= h0[5] ;
				offset <= offset + 1;
				state <= WRITE;
			end
			else if (offset == 6) begin
			
			
				cur_write_data <= h0[6];
				offset <= offset + 1;
				state <= WRITE;
			end
			else if (offset == 7) begin
			
			
				cur_write_data <= h0[7];
				offset <= offset + 1;
				state <= WRITE;
			end
		
			else begin
				state <= IDLE;
				done <= 1;			
			end
			*/
		end
	endcase
end

			
endmodule
