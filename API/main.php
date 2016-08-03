<?php
	include_once "Cmd.php";
	include_once "file.class.php";

	$data = null;
	switch ($_POST["task"])
	{
		case "solve":
			
			$txt = $_POST["data"];
			$num = mt_rand().".sm";
	
			$myfile = fopen($num, "w");
			
			for($i=0; $i<sizeof($txt); $i++){
				fwrite($myfile, $txt[$i]."\n");
			}
			
			fclose($myfile);
			$data = Cmd::solve($num);
			unlink($num);
			break;

		default:
			break;
	}

	echo json_encode($data); 
?>