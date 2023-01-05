#include <config.h>
#include <page_manager.h>


static void ExplorePageRun(void)
{
	/* 1. ��ʾҳ�� */

	/* 2. ����Prepare�߳� */

	/* 3. ����GetInputEvent��������¼����������� */
	while (1){
		
		InputEvent = ExplorePageGetInputEvent();
		switch (InputEvent)	{
			case "����":
			{
				/* �ж��Ƿ��Ѿ��Ƕ��� */
				if (IsTopLevel)
					return 0;
				else
				{
					/* ��ʾ��һ��Ŀ¼��ҳ�� */
				}
				
				break;
			}
			case "ѡ��":
			{
				/* �����ѡ��Ŀ¼ */
				if (IsSelectDir){
					/* ��ʾ��һ��Ŀ¼ */
				}else{
					/* ���浱ǰҳ�� */
					StorePage();
					/* ��ʾbrowseҳ�� */
					Page("browse")->Run();
					/* �ָ�֮ǰ��ҳ�� */
					RestorePage();
				}
				
				break;
			}
			case "��ҳ":
			{
				/* ��ʾ��һҳ�� */
				break;
			}		

			case "��ҳ":
			{
				/* ��ʾ��һҳ�� */
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


