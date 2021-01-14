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
$failed = mysqli_query($con, "SELECT COUNT(1) FROM `failed`");
$failed_row = mysqli_fetch_array($failed);
$failed_total = $failed_row[0];

?>
<!DOCTYPE html>
<html lang="en">
<head>
		<title><?php echo $name; ?> &bull; Failed Consoles</title>
		<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
		<meta http-equiv="X-UA-Compatible" content="IE=edge" />
		<meta name="viewport" content="width=device-width, initial-scale=1" />
		<link href="css/styles.css" rel="stylesheet" type="text/css" />
		<script type="text/javascript" src="js/plugins/jquery/jquery.min.js"></script>
		<script type="text/javascript" src="js/plugins/jquery/jquery-ui.min.js"></script>
		<script type="text/javascript" src="js/plugins/bootstrap/bootstrap.min.js"></script>
		<script type="text/javascript" src="js/plugins/mcustomscrollbar/jquery.mCustomScrollbar.min.js"></script>
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
	<body>
		<div class="page-container">
            <div class="page-navigation">
                
                <div class="profile">                    
                    <img src="img/samples/users/xbox.png"/>
                    <div class="profile-info">
                        <a class="profile-title"><?php echo $_SESSION['username']; ?></a>                     
                    </div>
                </div>
				<ul class="navigation">
					<li><a href="manageconsoles.php"><i class="fa fa-users"></i> Manage Consoles</a></li>
					<li  class="active"><a href="failed.php"><i class="fa fa-ban"></i> Failed Consoles</a></li>
					<li><a href="settings.php"><i class="fa fa-gear"></i> Settings</a></li>
					<li><a href="lib/logout.php"><i class="fa fa-gear"></i> Logout</a></li>
				</ul>
			</div>
			<div class="page-content">
				<div class="container">				
					<div class="row">
						<div class="col-md-10">
							<div class="panel panel-info">
								<div class="panel-heading">
									<h3 class="panel-title">Failed (Total: <?php echo $failed_total; ?>)</h3>
								</div>
								<div class="panel-body controls no-padding">
									<div class="block">
										<table cellpadding="0" cellspacing="0" width="100%" class="table table-bordered table-striped sortable">
											<thead>
												<tr>
													<th>ID</th>
													<th>CPUKey</th>
													<th>IP</th>
												</tr>
											</thead>
											<tbody>
											<?php
											$failed_result = mysqli_query($con, "SELECT * FROM `failed`");
											while($failed_rows2 = mysqli_fetch_array($failed_result))
											{
												echo'
												<tr>
												<td>
												'.$failed_rows2['id'].'
												</td>
												<td>
												'.$failed_rows2['cpukey'].'
												</td>
												<td>
												'.$failed_rows2['ip'].'
												</td>
                                                ';
											}
											?>
											</tbody>
										</table>
									</div>
								</div>
						    <br />
						</div>
			       </div>
			       </div>
            <div class="page-sidebar"></div>
        </div>
    </body>
</html>