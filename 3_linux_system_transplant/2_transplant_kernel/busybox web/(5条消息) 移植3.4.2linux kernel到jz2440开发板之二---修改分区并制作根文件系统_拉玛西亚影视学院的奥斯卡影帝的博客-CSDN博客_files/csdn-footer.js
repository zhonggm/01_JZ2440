"use strict";function _classCallCheck(e,n){if(!(e instanceof n))throw new TypeError("Cannot call a class as a function")}var _createClass=function(){function e(e,n){for(var t=0;t<n.length;t++){var a=n[t];a.enumerable=a.enumerable||!1,a.configurable=!0,"value"in a&&(a.writable=!0),Object.defineProperty(e,a.key,a)}}return function(n,t,a){return t&&e(n.prototype,t),a&&e(n,a),n}}();!function(){function e(e){var n=new Date;document.cookie=e+"="+escape("1")+";max-age=0;expires="+n.toGMTString()+";domain=.csdn.net;path=/"}function n(){void 0,t("adblock",{step:"install"}),function(){var e=new Date;e.setDate(e.getDate()+7),document.cookie="c_adb=1; expires="+e.toGMTString()+"; domain=csdn.net; path=/"}(),"function"==typeof window.csdn.insertcallbackBlock&&window.csdn.insertcallbackBlock()}function t(e,n){var t=window.location.protocol+"//statistic.csdn.net/";$.get(t+e,n)}var a=new Image;a.src="https://g.csdnimg.cn/ads/ads.gif",a.onload=function(){void 0,e("c_adb")},a.onerror=function(){n()}}(),function(){function e(){var e=document.querySelector('meta[name="csdnFooter"]');if(e){var n=JSON.parse(e.getAttribute("content"));return Object.assign({el:document.querySelector("body"),type:1},n)}}var n="http"===location.protocol.substr(0,4)?"":"https:",t=n+"//g.csdnimg.cn/common/csdn-footer/csdn-footer.css",a=n+"//g.csdnimg.cn/common/csdn-footer/images",r=(new Date).getFullYear();if(window.csdn=window.csdn||{},window.csdn.csdnFooter=window.csdn.csdnFooter||{},!csdn.trackad){var i=document.createElement("script");i.type="text/javascript",i.async=!0,i.src="https://g.csdnimg.cn/asdf/1.1.3/trackad.js";var l=document.getElementsByTagName("script")[0];l.parentNode.insertBefore(i,l)}var o=function(){function e(){var n=arguments.length>0&&void 0!==arguments[0]?arguments[0]:{},a=n.el,r=void 0===a?document.querySelector("body"):a,i=n.type,l=void 0===i?1:i;_classCallCheck(this,e),this.el=this.queryEl(r),this.setType(l),this.importCss(t),this.render()}return _createClass(e,[{key:"render",value:function(){3===this.type?this.renderVerticalFooter():this.renderHorizontalFooter()}},{key:"renderVerticalFooter",value:function(){var e='\n        <div id="csdn-copyright-footer" class="side'+(this.isDark()?" dark dark1":"")+'">\n          <div class="copyright-footer-contact">\n            <div class="work-time">联系我们（工作时间：8:30-22:00）</div>\n            <div class="work-time">400-660-0108<a class="link" href="mailto:webmaster@csdn.net" target="_blank">kefu@csdn.net</a><a class="link" href="https://csdn.s2.udesk.cn/im_client/?web_plugin_id=29181" target="_blank">在线客服</a></div>\n          </div>\n          <ul class="copyright-footer-middle">\n            <li><a href="//www.csdn.net/company/index.html#about" target="_blank">关于我们</a></li>\n            <li class="line"></li>\n            <li><a href="//www.csdn.net/company/index.html#recruit" target="_blank">招贤纳士</a></li>\n            <li class="line"></li>\n            <li><a href="//marketing.csdn.net/questions/Q2202181741262323995" target="_blank">商务合作</a></li>\n            <li class="line"></li>\n            <li><a href="//marketing.csdn.net/questions/Q2202181748074189855" target="_blank">寻求报道</a></li>\n          </ul>\n          <ul class="copyright-footer-info">\n            <li><a href="http://beian.miit.gov.cn/publish/query/indexFirst.action" rel="noreferrer" target="_blank">京ICP备19004658号</a></li>\n            <li><a href="https://csdnimg.cn/cdn/content-toolbar/csdn-ICP.png" target="_blank">经营性网站备案信息</a></li>\n            <li><img src="'+a+'/badge.png" alt=""><a href="http://www.beian.gov.cn/portal/registerSystemInfo?recordcode=11010502030143" rel="noreferrer" target="_blank">公安备案号11010502030143</a></li>\n            <li><a href="https://csdnimg.cn/release/live_fe/culture_license.png" rel="noreferrer" target="_blank">京网文〔2020〕1039-165号</a></li>\n            <li><a href="https://img-home.csdnimg.cn/images/20210414021142.jpg" target="_blank">营业执照</a></li>\n            <li class="compact">©1999-'+r+'北京创新乐知网络技术有限公司</li>\n            <li><a href="http://www.bjjubao.org/" target="_blank">北京互联网违法和不良信息举报中心</a></li>\n            <li><a href="https://download.csdn.net/tutelage/home" target="_blank">家长监护</a></li>\n            <li><a href="http://www.12377.cn/" target="_blank">中国互联网举报中心</a></li>\n            <li><a href="http://www.cyberpolice.cn/" target="_blank">网络110报警服务</a></li>\n            <li><a href="https://chrome.google.com/webstore/detail/csdn%E5%BC%80%E5%8F%91%E8%80%85%E5%8A%A9%E6%89%8B/kfkdboecolemdjodhmhmcibjocfopejo?hl=zh-CN" target="_blank">Chrome商店下载</a></li>\n            <li><a href="https://www.csdn.net/company/index.html#statement" target="_blank">版权与免责声明</a></li>\n            <li><a href="https://blog.csdn.net/blogdevteam/article/details/90369522" target="_blank">版权申诉</a></li>\n            <li><a href="https://img-home.csdnimg.cn/images/20220705052819.png" target="_blank">出版物许可证</a></li>\n          </ul>\n        </div>\n      ';$(this.el).append(e)}},{key:"renderHorizontalFooter",value:function(){var e='\n        <div id="copyright-box" class="'+(this.isDark()?"dark":"")+'">\n          <div id="csdn-copyright-footer" class="column'+(2===this.type?" small":"")+(this.isDark()?" dark":"")+'">\n            <ul class="footer-column-t">\n            <li>\n              <a href="//www.csdn.net/company/index.html#about" target="_blank">关于我们</a>\n            </li>\n            <li>\n              <a href="//www.csdn.net/company/index.html#recruit" target="_blank">招贤纳士</a>\n            </li>\n            <li><a href="//marketing.csdn.net/questions/Q2202181741262323995" target="_blank">商务合作</a></li>\n            <li><a href="//marketing.csdn.net/questions/Q2202181748074189855" target="_blank">寻求报道</a></li>\n            <li>\n              <img src="'+a+'/tel.png" alt="">\n              <span>400-660-0108</span>\n            </li>\n            <li>\n              <img src="'+a+'/email.png" alt="">\n              <a href="mailto:webmaster@csdn.net" target="_blank">kefu@csdn.net</a>\n            </li>\n            <li>\n              <img src="'+a+'/cs.png" alt="">\n              <a href="https://csdn.s2.udesk.cn/im_client/?web_plugin_id=29181" target="_blank">在线客服</a>\n            </li>\n            <li>\n              工作时间&nbsp;8:30-22:00</a>\n            </li>\n          </ul>\n            <ul class="footer-column-b">\n            <li><img src="'+a+'/badge.png" alt=""><a href="http://www.beian.gov.cn/portal/registerSystemInfo?recordcode=11010502030143" rel="noreferrer" target="_blank">公安备案号11010502030143</a></li>\n            <li><a href="http://beian.miit.gov.cn/publish/query/indexFirst.action" rel="noreferrer" target="_blank">京ICP备19004658号</a></li>\n            <li><a href="https://csdnimg.cn/release/live_fe/culture_license.png" rel="noreferrer" target="_blank">京网文〔2020〕1039-165号</a></li>\n            <li><a href="https://csdnimg.cn/cdn/content-toolbar/csdn-ICP.png" target="_blank">经营性网站备案信息</a></li>\n            <li><a href="http://www.bjjubao.org/" target="_blank">北京互联网违法和不良信息举报中心</a></li>\n            <li><a href="https://download.csdn.net/tutelage/home" target="_blank">家长监护</a></li>\n            <li><a href="http://www.cyberpolice.cn/" target="_blank">网络110报警服务</a></li>\n            <li><a href="http://www.12377.cn/" target="_blank">中国互联网举报中心</a></li>\n            <li><a href="https://chrome.google.com/webstore/detail/csdn%E5%BC%80%E5%8F%91%E8%80%85%E5%8A%A9%E6%89%8B/kfkdboecolemdjodhmhmcibjocfopejo?hl=zh-CN" target="_blank">Chrome商店下载</a></li>\n            <li>©1999-'+r+'北京创新乐知网络技术有限公司</li>\n            <li><a href="https://www.csdn.net/company/index.html#statement" target="_blank">版权与免责声明</a></li>\n            <li><a href="https://blog.csdn.net/blogdevteam/article/details/90369522" target="_blank">版权申诉</a></li>\n            <li><a href="https://img-home.csdnimg.cn/images/20220705052819.png" target="_blank">出版物许可证</a></li>\n            <li><a href="https://img-home.csdnimg.cn/images/20210414021142.jpg" target="_blank">营业执照</a></li>\n          </ul>\n          </div>\n        </div>\n      ';$(this.el).append(e)}},{key:"queryEl",value:function(e){if("string"==typeof e){var n=document.querySelector(e);return n||void 0,n}return e}},{key:"setType",value:function(e){1==e||2==e||3==e?this.type=Number(e):void 0}},{key:"isDark",value:function(){var e=document.querySelector('meta[name="toolbar"]'),n=e&&JSON.parse(e.getAttribute("content")).type;return window.csdn.toolbarIsBlack||"1"===n}},{key:"importCss",value:function(e){var n=document.createElement("link");n.rel="stylesheet",n.type="text/css",n.href=e,document.getElementsByTagName("head")[0].appendChild(n)}}]),e}();$(function(){var n=e();new o(n||window.csdn.csdnFooter.options)})}();