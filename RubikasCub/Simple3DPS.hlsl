
struct Input
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

struct Output
{
	float4 color : SV_TARGET;
};

Output main(Input input) {
	Output output;
	output.color = input.color;
	return output;
}