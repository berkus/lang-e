

foreach $size (16,32,64,128,512) {
	$times{"sdcg$i"} = &run("sdcg", $size);
	$times{"static$i"} = &run("static", $size);
	$times{"dcg$i"} = &run("dcg", $size);
}

sub run {
	local($name, $size) = @_;
	local(@times);

	$output = `$name -c 4096 -t 3 -n $size`;
	@trials = split(/\s+/, $output);
	$n = ($#trials + 1) / 2;
	for($i = 0; $i < $n; $i++) {
		$times[$i] = $trials[2*$i] + $trials[2*$i+1];
	}
	@times = sort @times;
	print "$name\t$size\t", $times[$n/2], "\n";
}
