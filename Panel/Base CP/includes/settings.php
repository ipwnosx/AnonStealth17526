<?php
include "database.php";

if(!isset($_SESSION))
{
	session_start();
}

$getsettings = mysqli_query($con, "SELECT * FROM `settings` LIMIT 1") or die(mysqli_error($con));

while($row = mysqli_fetch_assoc($getsettings))
{
	//you can add more settings here
	//
	//$yoursettingname = $row['yoursettingsnameindb'];
	$name = $row['name'];
}
?>