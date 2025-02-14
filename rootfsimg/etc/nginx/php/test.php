<?php
$str = "World!Helloo";
$pattern = '/Helloo/';
if (preg_match($pattern, $str, $matches)) {
	    echo "YES!" . PHP_EOL;
	        print_r($matches);
} else {
	    echo "NO!";
}
?>
