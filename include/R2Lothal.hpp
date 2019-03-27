/*
 * R2Lothal.hpp
 * Lothal
 *
 * Created by Zhu Yongjian on 2019/1/11
 * Copyright ? 2018 rokid. All rights reserved.
 */

/** \file R2Lothal.hpp
 * \brief 声明 SDK Lothal 接口类
 */

#ifndef _R2LOTHAL_HPP_
#define _R2LOTHAL_HPP_

#include <string>
#include "R2Types.hpp"
#include "R2Event.hpp"

namespace lothal {

/** \class R2Lothal
 * \brief R2Lothal 是 SDK Lothal 对外的接口层，它提供了：
 * \li 基本参数设置及查询：比如 MIC 通道、采样率等
 * \li pipeline 管理：启动、停止、暂停及恢复
 * \li 事件管理：注册事件回调，事件上报等
 * \li 业务 API：添加与删除激活词等
 */
class R2Lothal {

public:
	/** \fn R2Lothal()
	 * \brief R2Lothal 默认构造函数。实例化之后需要根据需要
	 * 调用 API 设置正确的 MIC 通道及采样率等参数。
	 *
	 * \note 默认 pipeline 中不包含任何处理模块，需要调用 API
	 * addModule() 往 pipeline 中增加需要的处理模块。
	 */
	R2Lothal();
	/** \fn R2Lothal(const std::string &config)
	 * \brief R2Lothal 构造函数。配置文件已正确地配置好基本
	 * 参数（比如 MIC 通道及采样率等）及 pipeline 中所包含的
	 * 处理模块，不需要再重新 addModule()。
	 * \param[in] config Lothal 配置文件路径，具体配置内容可参考
	 * 用户配置手册。
	 */
	R2Lothal(const std::string &config);
	/** \fn ~R2Lothal()
	 * \brief R2Lothal 析构函数，将释放 R2Lothal 实例所分配的
	 * 所有资源（包括 pipeline 及其中的处理模块等）。
	 */
	virtual ~R2Lothal();

public:
	/** \fn const char* version()
	 * \brief 获取 SDK 版本信息
	 * \return 返回版本信息字符串。
	 */
	static const char* version();

	/** \fn int getParams(int &channel, R2BitFormat &format, int &rate)
	 * \brief 查询基本参数信息
	 * \param[out] channel 保存返回的 MIC 通道
	 * \param[out] format 保存返回的数据格式
	 * \param[out] rate 保存返回的采样率
	 * \return 成功返回 \link R2_EOK \endlink，失败返回值小于0。
	 */
	int getParams(int &channel, R2BitFormat &format, int &rate);

	/** \fn int setParams(int channel, R2BitFormat format, int rate)
	 * \brief 设置基本参数信息
	 * \param[in] channel MIC 通道数
	 * \param[in] format 数据格式
	 * \param[in] rate 采样率
	 * \return 成功返回 \link R2_EOK \endlink，失败返回值小于0。
	 * \retval -R2_EWRONGARGS 输入参数有误
	 */
	int setParams(int channel, R2BitFormat format, int rate);

	/** \fn int setMicShape(R2MicArrayShape shape)
	 * \brief 设置 MIC 外形。
	 * \param[in] shape MIC 外形，支持类型见 \link R2MicArrayShape \endlink。
	 * \return 成功返回 \link R2_EOK \endlink，失败返回值小于0。
	 * \retval -R2_EWRONGARGS 输入参数有误。
	 */
	int setMicShape(R2MicArrayShape shape);

	/** \fn R2MicArrayShape getMicShape()
	 * \brief 获取已设置的 MIC 外形。
	 * \return 返回 MIC 外形。
	 */
	R2MicArrayShape getMicShape();

	/** \fn int getDataBlockSize()
	 * \brief 获取数据块大小。process() 函数每次处理的数据长
	 * 度必须为块大小。
	 * \return 返回获取的块字节数。
	 */
	int getDataBlockSize();

	/** \fn int getMaxAngel()
	 * \brief 获取设备最大角度。最大角度值与 MIC 外形有关，
	 * 线麦为180度，环麦为360度。
	 * \return 返回获取的最大角度。
	 */
	int getMaxAngel();

	/** \fn int addModule(const std::string &provider, const std::string &name)
	 * \brief 往 pipeline 上增加处理模块。当前支持的模块有
	 * IN, AEC, ENGINE 及 VAD，详细请见模块配置。
	 * \param[in] provider 当前未使用该参数，可传入空字符串。
	 * \param[in] name 待加入 pipeline 的模块名。
	 * \return 执行成功返回 \link R2_EOK \endlink，失败返回值小于0。
	 * \retval -R2_EEXIST 该模块已存在。
	 * \note 增加模块时，pipeline 必须先处于非工作状态，
	 * 可以先调用 stopPipeline() 停止pipeline。
	 */
	int addModule(const std::string &provider, const std::string &name);

	/** \fn int removeModule(const std::string &name)
	 * \brief 从 pipeline 中移除该处理模块。
	 * \param[in] name 待移除的模块名。
	 * \return 执行成功返回 \link R2_EOK \endlink，失败返回值小于0。
	 * \note 移除模块时，pipeline 必须先处于非工作状态，
	 * 可以先调用 stopPipeline() 停止pipeline。
	 */
	int removeModule(const std::string &name);

	/** \fn bool isModuleEnabled(const std::string &name)
	 * \brief 判断该模块是否被启用。
	 * \param[in] name 模块名。
	 * \return 如果该模块已启用，则返回true，否则返回false。
	 */
	bool isModuleEnabled(const std::string &name);

	/** \fn int startPipeline()
	 * \brief 启动 pipeline。只有启动 pipeline 之后，才能开始
	 * 处理输入的语音数据。
	 * \return 成功返回 \link R2_EOK \endlink，失败返回值小于0。
	 * \retval -R2_ESYSTEM 可能是处理模块启动异常，比如该模块不
	 * 支持，或者系统可用内存不足等。
	 * \retval -R2_ENOTSUPPORT 不支持该操作，可能是因为 pipeline
	 * 未增加任何处理模块，此时应该调用 addModule() 增加所
	 * 需要的处理模块。
	 */
	int startPipeline();

	/** \fn int stopPipeline()
	 * \brief 停止 pipeline。这将停此一切语音数据的处理，并
	 * 释放每个模块所分配的资源。
	 * \return 成功则返回 \link R2_EOK \endlink。
	 * \note 如果需要重新启动，只需要调用 startPipeline()，
	 不需要再调用 addModule()。
	 */
	int stopPipeline();

	/** \fn int pausePipeline()
	 * \brief 暂停 pipeline 的运行，暂停语音数据的处理。但
	 * 不释放每个模块分配的资源。
	 * \return 成功返回 \link R2_EOK \endlink。
	 */
	int pausePipeline();

	/** \fn int resumePipeline()
	 * \brief 恢复被暂停的 pipeline。
	 * \return 成功返回 \link R2_EOK \endlink。
	 */
	int resumePipeline();

	/** \fn int restartPipeline()
	 * \brief 重新启动 pipeline，释放并重新创建每个 pipeline
	 * 中的处理模块。
	 * \return 成功返回 \link R2_EOK \endlink，失败返回值小于0。
	 */
	int restartPipeline();

	/** \fn bool isReady()
	 * \brief 判断当前 pipeline 是否处于工作状态。
	 * \return 如果已被调用 stopPipeline() 停止运行，或者调用
	 * resumePipeline() 暂停运行，则返回 False，否则返回 True。
	 */
	bool isReady();

	/** \fn int process(const unsigned char* data, int size)
	 * \brief 往 pipeline 输入语音数据流，并进行处理。语音
	 * 数据处理过程中会产生一系列事件，用户需要捕捉这些
	 * 事件并处理，详细请见事件处理章节。
	 * \param[in] data 待处理的数据流。
	 * \param[in] size 待处理数据流字节长度。
	 * \return 成功返回 \link R2_EOK \endlink，失败返回值小于0。
	 * \retval -R2_ENOTREADY pipeline 未准备就绪，可能是因为尚示
	 * 调用 startPipeline() 启动 pipeline。
	 * \retval -R2_EWRONGARGS 语音流在处理过程中出现了错误，导
	 * 致数据信息不正确。
	 */
	int process(const unsigned char* data, int size);

	/** \fn void reportEvent(const std::string &name, void *arg)
	 * \brief 上报事件
	 * \param[in] name 事件名
	 * \param[in] arg 事件需要携带的额外信息
	 * \return void
	 */
	void reportEvent(const std::string &name, void *arg);

	/** \fn int setEventHandler(const std::string &name, R2EventHandlerSp handler)
	 * \brief 设置事件的处理
	 * \param[in] name 待处理的事件名
	 * \param[in] handler 事件的处理对象
	 * \return 成功返回 \link R2_EOK \endlink，失败返回值小于0。
	 * \retval -R2_EEXIST 表示该事件处理对象已设置捕捉处理该事件
	 */
	int setEventHandler(const std::string &name,
			R2EventHandlerSp handler);

	/** \fn void removeEventHandler(const std::string &name, R2EventHandlerSp handler)
	 * \brief 移除对该事件的处理。
	 * \param[in] name 待处理的事件名
	 * \param[in] handler 事件的处理对象
	 * \return void
	 */
	void removeEventHandler(const std::string &name,
			R2EventHandlerSp handler);

	/** \fn int addWord(const std::string &content, const std::string &pinyin, int type)
	 * \brief 添加自定义激活词。
	 * \param[in] content 激活词内容，比如"若琪"。
	 * \param[in] pinyin 激活词音，比如"ruoqi"。
	 * \param[in] type 激活词类型，值为1。
	 * \return 成功返回 \link R2_EOK \endlink，失败返回值小于0。
	 */
	int addWord(const std::string &content, const std::string &pinyin, int type);

	/** \fn int addWord(const std::string &content, const std::string &pinyin, const int type, const float margin)
	 * \brief 添加自定义激活词。
	 * \param[in] content 激活词内容，比如"若琪"。
	 * \param[in] pinyin 激活词音，比如"ruoqi"。
	 * \param[in] type 激活词类型，值为1。
	 * \param[in] margin 激活词激活阈值，值越大表示对语音要求
	 * 越高，也就越难激活，经验值范围是[-2.0, 2.0]。
	 * \return 成功返回 \link R2_EOK \endlink，失败返回值小于0。
	 */
	int addWord(const std::string &content, const std::string &pinyin,
			    const int type, const float margin);

	/** \fn int addWord(const std::string &content, const std::string &info)
	 * \brief 添加自定义激活词。
	 * \param[in] content 激活词内容，比如"若琪"。
	 * \param[in] info
	 * \return 成功返回 \link R2_EOK \endlink，失败返回值小于0。
	 */
	int addWord(const std::string &content, const std::string &info);

	/** \fn int removeWord(const std::string &content)
	 * \brief 删除激活词
	 * \param[in] content 待删除的激活词。
	 * \return 成功返回 \link R2_EOK \endlink，失败返回值小于0。
	 */
	int removeWord(const std::string &content);

	/** \fn int getAllWords(std::string &words_string)
	 * \brief 获取所有激活词
	 * \param[out] words_string 用于保存返回得到的所有激活词
	 * \return 成功返回 \link R2_EOK \endlink，失败返回值小于0。
	 */
	int getAllWords(std::string &words_string);

	/** \fn int getWordInfo(const std::string &content, std::string &word_info)
	 * \brief 获取激活词信息
	 * \param[in] content 待查询的激活词，比如“若琪”
	 * \param[out] word_info 用于保存返回得到的激活词信息，比如
	 * 激活词内容、类型等。
	 * \return 成功返回 \link R2_EOK \endlink，失败返回值小于0。
	 */
	int getWordInfo(const std::string &content, std::string &word_info);

	/** \fn int setVoipStatus(bool enable)
	 * \brief 设置Voip的通话状态
	 * \param[in] enable 使能/去使能通话
	 * \param[in] source 通话数据来源，可选择AEC或BF后的数据
	 * \deprecated
	 */
	int setVoipStatus(bool enable, const std::string &source = R2_VOPI_SOURCE_AEC);

	/** \fn int setPickUpStatus(bool enable)
	 * \deprecated
	 */
	int setPickUpStatus(bool enable);

	/** \fn int forceTrigger()
	 * \brief 跳过激活词检查而进行强制激活，仅单次生效，语音输入完毕
	 * 则结束当前语音会话。需要重新调用该接口以实现强制激活。
	 * \return 成功返回 \link R2_EOK \endlink，失败返回其他值。
	 */
	int forceTrigger();

private:
	void *pimpl_ = NULL;
};

}

#endif /* _R2LOTHAL_HPP_ */

