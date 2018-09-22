cbuffer PerFrame : register(b0) {
	matrix view;
	matrix proj;
};

cbuffer PerModel : register(b1) {
	matrix world;
};

struct Input
{
	float4 pos : POSITION;
	float4 color : COLOR;
};

struct Output
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};


Output main(Input input)
{
	Output output;
	input.pos.w = 1.0f;
	output.pos = mul(input.pos, world);
	output.pos = mul(output.pos, view);
	output.pos = mul(output.pos, proj);

	output.color = input.color;

	return output;
}