<?php
$DB_HOST = "localhost";
$DB_NAME = "base";
$DB_USER = "root";
$DB_PASS = "";

$con = mysqli_connect($DB_HOST, $DB_USER, $DB_PASS, $DB_NAME);

if(mysqli_connect_errno())
{
	die("Not Connection To MySQL Database " . mysqli_connect_error());
}
?>