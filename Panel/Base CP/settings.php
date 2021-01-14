<?php 
include 'includes/database.php';
include 'includes/settings.php';

if(!isset($_SESSION))
{
	session_start();
}

if(!isset($_SESSION['username']))
{
	echo'
	<script language="javascript">
	window.location.href="index.php"
	</script>
	';
}
?>
<!DOCTYPE html>
<html lang="en">
<head>
		<title><?php echo $name; ?> &bull; Settings</title>
		<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
		<meta http-equiv="X-UA-Compatible" content="IE=edge" />
		<meta name="viewport" content="width=device-width, initial-scale=1" />
		<link href="css/styles.css" rel="stylesheet" type="text/css" />
		<link rel="icon" type="image/ico" href="FavIco/favicon.ico">
		<script type="text/javascript" src="js/plugins/jquery/jquery.min.js"></script>
		<script type="text/javascript" src="js/plugins/jquery/jquery-ui.min.js"></script>
		<script type="text/javascript" src="js/plugins/bootstrap/bootstrap.min.js"></script>
		<script src="https://maps.googleapis.com/maps/api/js?v=3.exp&amp;sensor=false&amp;libraries=places"></script>
		<script type="text/javascript" src="js/plugins/fancybox/jquery.fancybox.pack.js"></script>
		<script type="text/javascript" src="js/plugins/rickshaw/d3.v3.js"></script>
		<script type="text/javascript" src="js/plugins/rickshaw/rickshaw.min.js"></script>
		<script type="text/javascript" src="js/plugins/knob/jquery.knob.js"></script>
		<script type="text/javascript" src="js/plugins/daterangepicker/moment.min.js"></script>
		<script type="text/javascript" src="js/plugins/daterangepicker/daterangepicker.js"></script> 
		<script type="text/javascript" src="js/plugins/jvectormap/jquery-jvectormap-1.2.2.min.js"></script>
		<script type="text/javascript" src="js/plugins/jvectormap/jquery-jvectormap-europe-mill-en.js"></script>
		<script type="text/javascript" src="js/plugins/noty/jquery.noty.js"></script>
		<script type="text/javascript" src="js/plugins/noty/layouts/topCenter.js"></script>
		<script type="text/javascript" src="js/plugins/noty/layouts/topLeft.js"></script>
		<script type="text/javascript" src="js/plugins/noty/layouts/topRight.js"></script>    
		<script type="text/javascript" src="js/plugins/noty/themes/default.js"></script>
		<script type="text/javascript" src="js/plugins.js"></script>
		<script type="text/javascript" src="js/demo.js"></script>
		<script type="text/javascript" src="js/maps.js"></script>
		<script type="text/javascript" src="js/charts.js"></script>
		<script type="text/javascript" src="js/actions.js"></script>
		<script type="text/javascript" src="js/plugins/tagsinput/jquery.tagsinput.min.js"></script>
		<script type="text/javascript" src="js/plugins/icheck/jquery.icheck.min.js"></script>
		<script type="text/javascript" src="js/plugins/jquery/jquery-ui-timepicker-addon.js"></script>
		<script type="text/javascript" src="js/plugins/chained/jquery.chained.min.js"></script>
		<script type="text/javascript" src="js/plugins/datatables/jquery.dataTables.min.js"></script>
		<script type="text/javascript" src="js/plugins/select2/select2.min.js"></script>
		<script type="text/javascript" src="js/plugins/highlight/jquery.highlight-4.js"></script>
		<script type="text/javascript" src="js/plugins/other/faq.js"></script>
		<script type="text/javascript" src="js/plugins/summernote/summernote.min.js"></script>
		<script type="text/javascript" src="js/plugins/codemirror/codemirror.js"></script>
		<script type='text/javascript' src="js/plugins/codemirror/addon/edit/matchbrackets.js"></script>
		<script type='text/javascript' src="js/plugins/codemirror/mode/htmlmixed/htmlmixed.js"></script>
		<script type='text/javascript' src="js/plugins/codemirror/mode/xml/xml.js"></script>
		<script type='text/javascript' src="js/plugins/codemirror/mode/javascript/javascript.js"></script>
		<script type='text/javascript' src="js/plugins/codemirror/mode/css/css.js"></script>
		<script type='text/javascript' src="js/plugins/codemirror/mode/clike/clike.js"></script>
		<script type='text/javascript' src="js/plugins/codemirror/mode/php/php.js"></script>
		</head>
	<body>
		<div class="page-container">
			<div class="page-navigation">
				<div class="profile">
					<img src="img/samples/users/xbox.png"/>
					<div class="profile-info">
						<a href="javascript:void(0);" class="profile-title"><?php echo $_SESSION['username']; ?> </a>
					</div>
				</div>
				<ul class="navigation">
					<li><a href="manageconsoles.php"><i class="fa fa-users"></i> Manage Consoles</a></li>
					<li><a href="failed.php"><i class="fa fa-ban"></i> Failed Consoles</a></li>
					<li  class="active"><a href="settings.php"><i class="fa fa-cog"></i> Settings</a></li>
					<li><a href="lib/logout.php"><i class="fa fa-gear"></i> Logout</a></li>
		
				</ul>
			</div>
			<div class="page-content">
				<div class="container">	
					<?php
					if(isset($_POST['clear_clients']))
					{
						$clear_clients = mysqli_query($con, "TRUNCATE `clients`");
						if($clear_clients)
						{
							echo '<div class="row"><div class="col-md-12"><div class="alert alert-success">
                                <button type="button" class="close" data-dismiss="alert" aria-hidden="true">×</button>
                                <strong>Well done!</strong> Clients cleared successfully.
                            </div></div></div>';
						}
						else
						{
							echo '<div class="row"><div class="col-md-12"><div class="alert alert-danger">
                                <button type="button" class="close" data-dismiss="alert" aria-hidden="true">×</button>
                                <strong>Fail!</strong> Clients cant be cleared, please try again.
                            </div></div></div>';
						}
					}
					?>
					<?php
					if(isset($_POST['clear_tokens']))
					{
						$clear_tokens = mysqli_query($con, "TRUNCATE `tokens`");
						if($clear_tokens)
						{
							echo '<div class="row"><div class="col-md-12"><div class="alert alert-success">
                                <button type="button" class="close" data-dismiss="alert" aria-hidden="true">×</button>
                                <strong>Well done!</strong> Tokens cleared successfully.
                            </div></div></div>';
						}
						else
						{
							echo '<div class="row"><div class="col-md-12"><div class="alert alert-danger">
                                <button type="button" class="close" data-dismiss="alert" aria-hidden="true">×</button>
                                <strong>Fail!</strong> Tokens cant be cleared, please try again.
                            </div></div></div>';
						}
					}
					?>
					<?php
					if(isset($_POST['updsettings']))
					{
					$servername = $_POST['servername'];
					$changesettings = mysqli_query($con, "UPDATE `settings` SET `name` = '".$servername."'");
					if($changesettings)
					{
						echo '<div class="row"><div class="col-md-12"><div class="alert alert-success">
                                <button type="button" class="close" data-dismiss="alert" aria-hidden="true">×</button>
                                <strong>SUCCESS</strong> Server name changed!
                            </div></div></div>';
					}
					elseif(!$changesettings)
					{
						echo '<div class="row"><div class="col-md-12"><div class="alert alert-danger">
                                <button type="button" class="close" data-dismiss="alert" aria-hidden="true">×</button>
                                <strong>FAIL</strong> Server name did not change!
                            </div></div></div>';
					}
				    }
					?>
					<?php
					if(isset($_POST['clear_failedlogins']))
					{
						$clear_failed = mysqli_query($con, "TRUNCATE `failed`");
						if($clear_failed)
						{
							echo'<div class="row"><div class="col-md-12"><div class="alert alert-success">
							<button type="submit" class="close" data-dismiss="alert" aria-hidden="true">x</button>
							<strong>OK!</strong> Failed logins cleared!.
							</div></div></div>
							';
						}
						elseif(!$clear_failed)
						{
							echo'<div class="row"><div class="col-md-12"><div class="alert alert-danger">
							<button type=submit" class="close" data-dismiss="alert" aria-hidden="true">x</button>
							<strong>Fail!</strong> Failed logins cant be cleared, Try Again!.
							</div></div></div>
							';
						}
					}
					?>
					<?php
					if(isset($_POST['updadmin']))
					{
						$admin_name = $_POST['adminnameVal'];
						$admin_pass = openssl_digest($_POST['adminpassVal'], "SHA512");

						$change_admin = mysqli_query($con, "UPDATE `users` SET `username`='".$admin_name."',`password`='".$admin_pass."' WHERE `id` = '".$_SESSION['id']."'");

						if($change_admin)
						{
							echo '<div class="row"><div class="col-md-12"><div class="alert alert-success">
							<button type="submit" class = "close" data-dismiss="alert" aria-hidden="true">x</button>
							<strong>Well Done!</strong> Admin settings successfully changed.
							</div></div></div>';
						}
						elseif(!$change_admin)
						{
							echo '<div class="row"><div class="col-md-12"><div class="alert alert-danger">
							<button type="submit" class = "close" data-dismiss="alert" aria-hidden="true">x</button>
							<strong>Fail!</strong> Admin settings not changed.
							</div></div></div>';
						}
					}
					?>
					<div class="row">
					    <div class="col-md-12">
					        <div class="panel panel-info">
					            <form method="POST">
					                <div class="panel-heading">
					                    <h3 class="panel-title">Server Name</h3>
					                </div>
					                <div class="panel-body controls no-padding">
					                <div class="row-form">
                                            <div class="col-md-3"><strong>Server Name :</strong></div>
                                            <div class="col-md-9"><input type="text" required class="form-control" placeholder="Server Name" name="servername"/></div>
                                        </div>
					                </div>
					                <div class="panel-footer"><button class="btn btn-success" name="updsettings">Update Server Name</button></div>
					            </form>
					        </div>
					    </div>
					</div>	
					<div class="row">
					    <div class="col-md-12">
					        <div class="panel panel-info">
					            <form method="POST">
					                <div class="panel-heading">
					                    <h3 class="panel-title">Login Settings</h3>
					                </div>
					                <div class="panel-body controls no-padding">
					                <div class="row-form">
                                            <div class="col-md-3"><strong>Username :</strong></div>
                                            <div class="col-md-9"><input type="text" required class="form-control" placeholder="Admin Name" name="adminnameVal" value = "<?php echo $_SESSION['username']; ?>" /></div>
					                </div>
					                <div class="row-form">
                                            <div class="col-md-3"><strong>Password :</strong></div>
                                            <div class="col-md-9"><input type="password" required class="form-control" placeholder="Admin Password" name="adminpassVal"/></div>
                                        </div>
                                    </div>
					                <div class="panel-footer"><button class="btn btn-success" name="updadmin">Update Login Settings</button></div>
					            </form>
					        </div>
					    </div>
					</div>			
					<div class="row">
						<div class="col-md-12">
							<div class="panel panel-info">
								<form method="POST">
									<div class="panel-heading">
										<h3 class="panel-title">Other Settings</h3>
									</div>
									<div class="panel-body controls no-padding">
									</div>
									<div class="panel-footer">
									<button class="btn btn-success" name="reboot">Reboot Server</button>
									<button class="btn btn-success" name="clear_clients">Clear Clients</button>
									<button class="btn btn-success" name="clear_tokens">Clear Tokens</button>
									<button class="btn btn-success" name="clear_failedlogins">Clear Failed Logins</button></div>
								</form>
							</div>
						</div>
					</div>
				</div>
			</div>
		</div>
	</body>
</html>