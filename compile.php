<?php
require_once("php/_basic_tasks.php");
$user_os = getOS();
echo "<p>System = ".$user_os."</p>";
if(!file_exists("source")) {
	echo "<p>The ‘source’ folder is missing or misplaced. Follow instructions on page <a target=\"_blank\" href=\"https://bolprocessor.org/check-bp3/#install\">https://bolprocessor.org/check-bp3/</a> and check your installation!</p>";
	echo "<p><a href=\"php/index.php\">Return to Bol Processor home page</a></p>";
	die(); 
	}
if(file_exists("bp")) unlink("bp");
$this_file = "bp_compile_result.txt";
if(file_exists($this_file)) unlink($this_file);
echo "<p style=\"text-align:center;  width:50%;\">----------- compiling -----------</p>";
exec("make 2>bp_compile_result.txt");
if(file_exists($this_file)) {
	$content = trim(@file_get_contents($this_file,TRUE));
	if($content <> '') echo $content;
	else echo "<p style=\"text-align:center;  width:50%;\">Compilation worked!</p>";
	}
else echo "<p style=\"text-align:center;  width:50%;\">Compilation failed… Check the “source/BP2” folder!</p>";
echo "<p style=\"text-align:center;  width:50%;\">------ end of compilation ------</p>";
echo "<div style=\"background-color:azure; padding:6px; width:50%; text-align:center;\"><p>In case compilation failed (because the “make” command was not accepted)<br />you may need to install <a target=\"_blank\" href=\"https://www.cnet.com/tech/computing/install-command-line-developer-tools-in-os-x/\">command line developer tools in OS X</a>.</p><p>Recent versions of MacOS do it automatically and no further adjustment is required.</p></div>";
echo "<p><a href=\"php/index.php\">Return to Bol Processor home page</a></p>"; 
?>