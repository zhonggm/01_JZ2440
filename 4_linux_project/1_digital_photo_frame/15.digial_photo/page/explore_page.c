#include <config.h>
#include <page_manager.h>


static void ExplorePageRun(void)
{
	/* 1. 显示页面 */

	/* 2. 创建Prepare线程 */

	/* 3. 调用GetInputEvent获得输入事件，进而处理 */
	while (1){
		
		InputEvent = ExplorePageGetInputEvent();
		switch (InputEvent)	{
			case "向上":
			{
				/* 判断是否已经是顶层 */
				if (IsTopLevel)
					return 0;
				else
				{
					/* 显示上一个目录的页面 */
				}
				
				break;
			}
			case "选择":
			{
				/* 如果是选择目录 */
				if (IsSelectDir){
					/* 显示下一级目录 */
				}else{
					/* 保存当前页面 */
					StorePage();
					/* 显示browse页面 */
					Page("browse")->Run();
					/* 恢复之前的页面 */
					RestorePage();
				}
				
				break;
			}
			case "下页":
			{
				/* 显示下一页面 */
				break;
			}		

			case "上页":
			{
				/* 显示上一页面 */
				break;
			}		
		}
	}
}

static T_PageAction g_tExplorePageAction = {
	.name          = "explore",
	.Run           = ExplorePageRun,
	.GetInputEvent = ExplorePageGetInputEvent,
	.Prepare       = ExplorePagePrepare;
};

int ExplorePageInit(void)
{
	return RegisterPageAction(&g_tExplorePageAction);
}


