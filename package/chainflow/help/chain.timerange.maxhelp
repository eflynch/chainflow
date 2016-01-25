{
	"patcher" : 	{
		"fileversion" : 1,
		"appversion" : 		{
			"major" : 7,
			"minor" : 0,
			"revision" : 6,
			"architecture" : "x86",
			"modernui" : 1
		}
,
		"rect" : [ 34.0, 79.0, 1612.0, 933.0 ],
		"bgcolor" : [ 1.0, 1.0, 1.0, 1.0 ],
		"editing_bgcolor" : [ 1.0, 1.0, 1.0, 1.0 ],
		"bglocked" : 0,
		"openinpresentation" : 0,
		"default_fontsize" : 12.0,
		"default_fontface" : 0,
		"default_fontname" : "Courier",
		"gridonopen" : 1,
		"gridsize" : [ 15.0, 15.0 ],
		"gridsnaponopen" : 1,
		"objectsnaponopen" : 1,
		"statusbarvisible" : 2,
		"toolbarvisible" : 1,
		"lefttoolbarpinned" : 0,
		"toptoolbarpinned" : 0,
		"righttoolbarpinned" : 0,
		"bottomtoolbarpinned" : 0,
		"toolbars_unpinned_last_save" : 0,
		"tallnewobj" : 0,
		"boxanimatetime" : 200,
		"enablehscroll" : 1,
		"enablevscroll" : 1,
		"devicewidth" : 0.0,
		"description" : "",
		"digest" : "",
		"tags" : "",
		"style" : "chainflow",
		"subpatcher_template" : "",
		"boxes" : [ 			{
				"box" : 				{
					"id" : "obj-21",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 305.0, 286.0, 293.0, 18.0 ],
					"style" : "",
					"text" : "from timestamp to an hour layer"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-20",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 68.0, 284.0, 205.0, 20.0 ],
					"style" : "",
					"text" : "from 123234 to 123234 1hour"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-18",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 305.0, 259.0, 293.0, 18.0 ],
					"style" : "",
					"text" : "from one timestamp to another "
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-19",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 68.0, 259.0, 162.0, 20.0 ],
					"style" : "",
					"text" : "from 123234 to 234364"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-14",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 305.0, 146.0, 293.0, 18.0 ],
					"style" : "",
					"text" : "one hour starting from four hours ago"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-11",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 305.0, 168.0, 293.0, 18.0 ],
					"style" : "",
					"text" : "the last 1day + 1hour + 4sec"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-10",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 305.0, 189.5, 293.0, 18.0 ],
					"style" : "",
					"text" : "the last two hours"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-9",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 305.0, 235.0, 293.0, 18.0 ],
					"style" : "",
					"text" : "next three hours"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-7",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 305.0, 212.5, 293.0, 18.0 ],
					"style" : "",
					"text" : "two hours centered around last midnight"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-5",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 68.0, 343.0, 111.0, 20.0 ],
					"style" : "",
					"text" : "print @popup 1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-38",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 68.0, 235.0, 83.0, 20.0 ],
					"style" : "",
					"text" : "next 3hour"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-37",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 68.0, 211.5, 227.0, 20.0 ],
					"style" : "",
					"text" : "from mdnt -1hour to mdnt 1hour"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-36",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 68.0, 189.5, 169.0, 20.0 ],
					"style" : "",
					"text" : "from now -2hour to now"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-35",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 68.0, 146.0, 219.0, 20.0 ],
					"style" : "",
					"text" : "from now -4hour to now -3hour"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-31",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 68.0, 168.0, 155.0, 20.0 ],
					"style" : "",
					"text" : "last 1day 1hour 4sec"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-29",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 68.0, 315.0, 191.0, 20.0 ],
					"style" : "",
					"text" : "chain.timerange cachehelp"
				}

			}
, 			{
				"box" : 				{
					"bgcolor" : [ 0.0, 0.0, 0.0, 1.0 ],
					"fontface" : 1,
					"fontname" : "Courier",
					"fontsize" : 30.0,
					"id" : "obj-15",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 23.0, 145.0, 28.0, 36.0 ],
					"style" : "",
					"text" : "1",
					"textcolor" : [ 1.0, 1.0, 1.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-65",
					"linecount" : 13,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 14.5, 413.0, 480.0, 162.0 ],
					"style" : "",
					"text" : "Arguments:\n[chain.cache name]\n    name - optional argument shared by all chain\n           objects which associates each worker object\n           with a chain.site manager.\n\nAttributes:\n    tz   - timezone to measure midnight from in seconds from UTC\n\nMethods:\n    from [time] to [time]\n    last [time] - alias of from now -[time] to now\n    next [time] - alias of from now to now [time]\n"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-16",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 14.5, 55.0, 438.0, 18.0 ],
					"style" : "chainflow-subsubtitle",
					"text" : "Evan Lynch, MIT Media Lab - Responsive Environments © 2015"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-13",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 286.5, 21.0, 192.0, 26.0 ],
					"style" : "chainflow-subtitle",
					"text" : "chain.timerange"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-12",
					"linecount" : 2,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 2.5, 87.0, 513.0, 30.0 ],
					"style" : "",
					"text" : "The chain.timerange abstraction provides a convenient way to format a datetime range relative to now or to the previous midnight."
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-8",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 14.5, 7.0, 272.5, 54.0 ],
					"style" : "chainflow-title",
					"text" : "ChainFlow"
				}

			}
, 			{
				"box" : 				{
					"angle" : 268.301605,
					"bgcolor" : [ 0.0, 0.0, 0.0, 1.0 ],
					"id" : "obj-17",
					"maxclass" : "panel",
					"mode" : 0,
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 2.5, 4.0, 476.0, 79.0 ],
					"proportion" : 0.749053,
					"style" : "chainflow"
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"destination" : [ "obj-29", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-19", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-29", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-20", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-5", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-29", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-29", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-31", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-29", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"midpoints" : [ 77.5, 234.75, 77.5, 234.75 ],
					"source" : [ "obj-35", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-29", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-36", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-29", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-37", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-29", 0 ],
					"disabled" : 0,
					"hidden" : 0,
					"source" : [ "obj-38", 0 ]
				}

			}
 ],
		"dependency_cache" : [ 			{
				"name" : "chain.timerange.maxpat",
				"bootpath" : "~/things/chainflow/package/chainflow/patchers",
				"type" : "JSON",
				"implicit" : 1
			}
, 			{
				"name" : "chain.tdparse.maxpat",
				"bootpath" : "~/things/chainflow/package/chainflow/patchers",
				"type" : "JSON",
				"implicit" : 1
			}
, 			{
				"name" : "chain.time.mxo",
				"type" : "iLaX"
			}
 ],
		"embedsnapshot" : 0,
		"styles" : [ 			{
				"name" : "chainflow",
				"default" : 				{
					"fontface" : [ 0 ],
					"textjustification" : [ 0 ],
					"accentcolor" : [ 0.0, 0.0, 0.0, 1.0 ],
					"elementcolor" : [ 0.282353, 0.309804, 0.266667, 1.0 ],
					"color" : [ 0.960784, 0.827451, 0.156863, 1.0 ],
					"bgcolor" : [ 0.095481, 0.100396, 0.100293, 1.0 ],
					"textcolor_inverse" : [ 1.0, 1.0, 1.0, 1.0 ],
					"patchlinecolor" : [ 0.639216, 0.458824, 0.070588, 0.9 ],
					"fontname" : [ "Courier" ],
					"bgfillcolor" : 					{
						"type" : "color",
						"color" : [ 0.639216, 0.458824, 0.070588, 1.0 ],
						"color1" : [ 0.960784, 0.827451, 0.156863, 1.0 ],
						"color2" : [ 0.0, 0.0, 0.0, 1.0 ],
						"angle" : 270.0,
						"proportion" : 0.39,
						"autogradient" : 0
					}

				}
,
				"parentstyle" : "",
				"multi" : 0
			}
, 			{
				"name" : "chainflow-subsubtitle",
				"default" : 				{
					"textcolor" : [ 0.960784, 0.827451, 0.156863, 1.0 ]
				}
,
				"parentstyle" : "",
				"multi" : 0
			}
, 			{
				"name" : "chainflow-subtitle",
				"default" : 				{
					"fontsize" : [ 20.0 ],
					"textcolor" : [ 0.870588, 0.415686, 0.062745, 1.0 ]
				}
,
				"parentstyle" : "",
				"multi" : 0
			}
, 			{
				"name" : "chainflow-title",
				"default" : 				{
					"fontsize" : [ 48.0 ],
					"textcolor" : [ 0.960784, 0.827451, 0.156863, 1.0 ],
					"fontname" : [ "Courier Bold" ],
					"clearcolor" : [ 0.317647, 0.654902, 0.976471, 0.0 ]
				}
,
				"parentstyle" : "",
				"multi" : 0
			}
 ]
	}

}
