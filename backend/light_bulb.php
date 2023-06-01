<?php

if(isset($_POST["update_pwm"]) && is_numeric($_POST["update_pwm"]) && $_POST["update_pwm"] >= 0 && $_POST["update_pwm"] < 1024)
{
	file_put_contents("light_bulb_pwm.bin", $_POST["update_pwm"]);
}
else
{
	echo(file_get_contents("light_bulb_pwm.bin"));
}

?>