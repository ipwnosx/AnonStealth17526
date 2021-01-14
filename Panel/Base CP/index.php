<?php
include "includes/database.php";
include "includes/settings.php";

if(!isset($_SESSION))
{
    session_start();
}

if(isset($_SESSION['username']))
{
    echo '
    <script language ="javascript">
    window.location.href = "manageconsoles.php"
    </script>
    ';
}

?>
<!DOCTYPE html>
<html lang="en">
    
<meta http-equiv="content-type" content="text/html;charset=utf-8" />
<head>        
        <title><?php echo $name; ?> &bull; Login</title>    
        <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
        <meta http-equiv="X-UA-Compatible" content="IE=edge" />
        <meta name="viewport" content="width=device-width, initial-scale=1" />

        <link href="css/styles.css" rel="stylesheet" type="text/css" />
        <script type="text/javascript" src="js/plugins/jquery/jquery.min.js"></script>
        <script type="text/javascript" src="js/plugins/jquery/jquery-ui.min.js"></script>
        <script type="text/javascript" src="js/plugins/bootstrap/bootstrap.min.js"></script>
        <script type="text/javascript" src="js/plugins/mcustomscrollbar/jquery.mCustomScrollbar.min.js"></script>
                
        <script type="text/javascript" src="js/plugins/jquery-validation/jquery.validate.min.js"></script>

        <script type="text/javascript" src="js/plugins.js"></script>        
        <script type="text/javascript" src="js/demo.js"></script>
        <script type="text/javascript" src="js/actions.js"></script>        
        
    </head>
    <body>
        
        <div class="page-container">
            
            <div class="page-content page-content-default">

                <div class="block-login">
                    <div class="block-login-logo">
                        <a class="logo">Anon</a>
                    </div>     
                    <?php
                    if(isset($_POST['login']))
                    {
                        $username = mysqli_real_escape_string($con, htmlspecialchars($_POST['usernameVal']));
                        $pass = mysqli_real_escape_string($con, htmlspecialchars($_POST['passwordVal']));
                        $password = openssl_digest($pass, "MD5");

                        $result = mysqli_query($con, "SELECT * FROM `users` WHERE `username` = '$username' AND `password` = '$password'") or die(mysqli_error($con));
                        $row = mysqli_fetch_array($result);

                        $id = $row['id'];
                        $username = $row['username'];

                        $select_user = mysqli_query($con, "SELECT * FROM `users` WHERE `id` = '$id'") or die(mysqli_error($con));
                        $row2 = mysqli_fetch_array($select_user);

                        $user = $row2['username'];

                        if($username != $username)
                        {
                            echo '<div class="alert alert-danger">
                                    <button type="button" class="close" data-dismiss="alert" aria-hidden="true">X</button>
                                    <strong>Incorrect Credentials!</strong>
                                </div>
                                <br>';
                        }
                        else
                        {
                            $pass_check = mysqli_query($con, "SELECT * FROM `users` WHERE `username` = '$username' AND `id` = '$id'") or die(mysqli_error($con));
                            $row3 = mysqli_fetch_array($pass_check);

                            $select_pass = mysqli_query($con, "SELECT * FROM `users` WHERE `username` = '$username' AND `id` = '$id'") or die(mysqli_error($con));
                            $row4 = mysqli_fetch_array($select_pass);

                            $real_password = $row4['password'];

                            if($real_password != $password)
                            {
                                echo '<div class="alert alert-danger">
                                    <button type="button" class="close" data-dismiss="alert" aria-hidden="true">X</button>
                                    <strong>Incorrect Credentials!</strong>
                                </div>
                                <br>';
                            }
                            else
                            {
                                $_SESSION['id'] = $id;
                                $_SESSION['username'] = $username;

                                echo '<div class="alert alert-success">
                                    <button type="button" class="close" data-dismiss="alert" aria-hidden="true">X</button>
                                    <strong>Access Granted</strong> Redirecting..........
                                </div>
                                <br>';

                                sleep("2");
                                header("Location: manageconsoles.php");
                            }
                        }
                    }
                    ?>               
                    <div class="block-login-content">
                        <h1>Login</h1>
                        <form id="signinForm" method="POST" action="">
                            
                        <div class="form-group">                        
                            <input type="text" name="usernameVal" class="form-control" placeholder="username" value=""/>
                        </div>
                        <div class="form-group">                        
                            <input type="password" name="passwordVal" class="form-control" placeholder="password" value=""/>
                        </div>
                        <button name = "login" class="btn btn-primary btn-block" type="submit">Login</button>                                        
                        
                        </form>

                        <div class="sp"></div>
                        <div class="pull-center">
                            <center>Copyright © 2016 - <?php echo $name; ?> | All Rights Reserved.</center>
                        </div>
                    </div>

                </div>
                
            </div>
        </div>
        
        <script type="text/javascript">
        $("#signinForm").validate({
		rules: {
			usernameVal: "required",
			passwordVal: "required"
		},
		messages: {
			firstname: "Please enter your username",
			lastname: "Please enter your password"			
		}
	});            
        </script>
    </body>
</html>
