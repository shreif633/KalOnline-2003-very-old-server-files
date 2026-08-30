USE [master]
GO
/****** Object:  Database [kal_auth]    Script Date: 02.10.2018 13:52:01 ******/
IF NOT EXISTS (SELECT name FROM sys.databases WHERE name = N'kal_auth')
BEGIN
    CREATE DATABASE [kal_auth]
END
GO
ALTER DATABASE [kal_auth] SET ANSI_NULL_DEFAULT OFF 
GO
ALTER DATABASE [kal_auth] SET ANSI_NULLS OFF 
GO
ALTER DATABASE [kal_auth] SET ANSI_PADDING OFF 
GO
ALTER DATABASE [kal_auth] SET ANSI_WARNINGS OFF 
GO
ALTER DATABASE [kal_auth] SET ARITHABORT OFF 
GO
ALTER DATABASE [kal_auth] SET AUTO_CLOSE OFF 
GO
ALTER DATABASE [kal_auth] SET AUTO_SHRINK OFF 
GO
ALTER DATABASE [kal_auth] SET AUTO_UPDATE_STATISTICS ON 
GO
ALTER DATABASE [kal_auth] SET CURSOR_CLOSE_ON_COMMIT OFF 
GO
ALTER DATABASE [kal_auth] SET CURSOR_DEFAULT  GLOBAL 
GO
ALTER DATABASE [kal_auth] SET CONCAT_NULL_YIELDS_NULL OFF 
GO
ALTER DATABASE [kal_auth] SET NUMERIC_ROUNDABORT OFF 
GO
ALTER DATABASE [kal_auth] SET QUOTED_IDENTIFIER OFF 
GO
ALTER DATABASE [kal_auth] SET RECURSIVE_TRIGGERS OFF 
GO
ALTER DATABASE [kal_auth] SET AUTO_UPDATE_STATISTICS_ASYNC OFF 
GO
ALTER DATABASE [kal_auth] SET RECOVERY SIMPLE 
GO

ALTER DATABASE [kal_auth] SET ANSI_NULL_DEFAULT OFF 
GO
ALTER DATABASE [kal_auth] SET ANSI_NULLS OFF 
GO
ALTER DATABASE [kal_auth] SET ANSI_PADDING OFF 
GO
ALTER DATABASE [kal_auth] SET ANSI_WARNINGS OFF 
GO
ALTER DATABASE [kal_auth] SET ARITHABORT OFF 
GO
ALTER DATABASE [kal_auth] SET AUTO_CLOSE OFF 
GO
ALTER DATABASE [kal_auth] SET AUTO_SHRINK OFF 
GO
ALTER DATABASE [kal_auth] SET AUTO_UPDATE_STATISTICS ON 
GO
ALTER DATABASE [kal_auth] SET CURSOR_CLOSE_ON_COMMIT OFF 
GO
ALTER DATABASE [kal_auth] SET CURSOR_DEFAULT  GLOBAL 
GO
ALTER DATABASE [kal_auth] SET CONCAT_NULL_YIELDS_NULL OFF 
GO
ALTER DATABASE [kal_auth] SET NUMERIC_ROUNDABORT OFF 
GO
ALTER DATABASE [kal_auth] SET QUOTED_IDENTIFIER OFF 
GO
ALTER DATABASE [kal_auth] SET RECURSIVE_TRIGGERS OFF 
GO
ALTER DATABASE [kal_auth] SET  DISABLE_BROKER 
GO
ALTER DATABASE [kal_auth] SET AUTO_UPDATE_STATISTICS_ASYNC OFF 
GO
ALTER DATABASE [kal_auth] SET DATE_CORRELATION_OPTIMIZATION OFF 
GO
ALTER DATABASE [kal_auth] SET TRUSTWORTHY OFF 
GO
ALTER DATABASE [kal_auth] SET ALLOW_SNAPSHOT_ISOLATION OFF 
GO
ALTER DATABASE [kal_auth] SET PARAMETERIZATION SIMPLE 
GO
ALTER DATABASE [kal_auth] SET READ_COMMITTED_SNAPSHOT OFF 
GO
ALTER DATABASE [kal_auth] SET HONOR_BROKER_PRIORITY OFF 
GO
ALTER DATABASE [kal_auth] SET RECOVERY SIMPLE 
GO
ALTER DATABASE [kal_auth] SET  MULTI_USER 
GO
ALTER DATABASE [kal_auth] SET PAGE_VERIFY CHECKSUM  
GO
ALTER DATABASE [kal_auth] SET DB_CHAINING OFF 
GO
ALTER DATABASE [kal_auth] SET FILESTREAM( NON_TRANSACTED_ACCESS = OFF ) 
GO
ALTER DATABASE [kal_auth] SET TARGET_RECOVERY_TIME = 0 SECONDS 
GO
USE [kal_auth]
GO
/****** Object:  Table [dbo].[AuthStatistics]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[AuthStatistics](
	[RegDate] [smalldatetime] NULL,
	[Server] [smallint] NULL,
	[Knight] [smallint] NULL,
	[Mage] [smallint] NULL,
	[Archer] [smallint] NULL,
	[TotalUser] [smallint] NULL,
	[FishTrap] [smallint] NULL,
	[PrivateShop] [smallint] NULL,
	[Currency] [int] NULL
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[CLogin]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CLogin](
	[CID] [int] NOT NULL,
	[Type] [tinyint] NOT NULL,
	[ExpTime] [int] NOT NULL,
 CONSTRAINT [PK_CLogin] PRIMARY KEY CLUSTERED 
(
	[CID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[CNum]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CNum](
	[UID] [int] NOT NULL,
	[Num] [int] NOT NULL,
 CONSTRAINT [PK_CNum] PRIMARY KEY CLUSTERED 
(
	[UID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[ExpTable]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ExpTable](
	[Level] [tinyint] NOT NULL,
	[Exp] [bigint] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[GuildRank]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildRank](
	[Server] [tinyint] NOT NULL,
	[Name] [varchar](16) NOT NULL,
	[MemberNum] [tinyint] NOT NULL,
	[Exp] [bigint] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[IP]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[IP](
	[CID] [int] NOT NULL,
	[StartIP] [int] NOT NULL,
	[EndIP] [int] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[ItemBuy]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ItemBuy](
	[Num] [int] IDENTITY(1,1) NOT NULL,
	[Server] [tinyint] NOT NULL,
	[Userid] [nvarchar](12) NOT NULL,
	[billNum] [char](30) NULL,
	[ItemCode] [smallint] NOT NULL,
	[billCode] [int] NOT NULL,
	[quantity] [smallint] NOT NULL,
	[Unit] [varchar](50) NOT NULL,
	[Discount] [smallint] NOT NULL,
	[BuyDate] [datetime] NOT NULL,
	[Place] [bit] NOT NULL,
	[charDate] [char](10) NOT NULL,
	[Flag] [tinyint] NOT NULL,
	[Zipcode] [varchar](10) NOT NULL,
	[Gender] [bit] NOT NULL,
	[bSuccess] [bit] NOT NULL,
	[ErrNum] [varchar](20) NULL,
	[Age] [tinyint] NOT NULL,
	[Price] [int] NOT NULL,
	[giftid] [nvarchar](12) NULL,
	[msg] [nvarchar](50) NULL,
 CONSTRAINT [PK_ItemBuy] PRIMARY KEY CLUSTERED 
(
	[BuyDate] DESC,
	[Userid] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[ItemBuy_backup]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ItemBuy_backup](
	[Num] [int] NOT NULL,
	[Server] [tinyint] NOT NULL,
	[Userid] [nvarchar](12) NOT NULL,
	[billNum] [char](30) NULL,
	[ItemCode] [smallint] NOT NULL,
	[billCode] [int] NOT NULL,
	[quantity] [smallint] NOT NULL,
	[Unit] [varchar](50) NOT NULL,
	[Discount] [smallint] NOT NULL,
	[BuyDate] [datetime] NOT NULL,
	[Place] [bit] NOT NULL,
	[charDate] [char](10) NOT NULL,
	[Flag] [tinyint] NOT NULL,
	[Zipcode] [varchar](10) NOT NULL,
	[Gender] [bit] NOT NULL,
	[bSuccess] [bit] NOT NULL,
	[ErrNum] [varchar](20) NULL,
	[Age] [tinyint] NOT NULL,
	[Price] [int] NOT NULL,
	[giftid] [nvarchar](12) NULL,
	[msg] [nvarchar](50) NULL
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[ItemDetail]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ItemDetail](
	[billCode] [int] NOT NULL,
	[ItemCode] [smallint] NOT NULL,
	[Unit] [varchar](50) NOT NULL,
	[Duration] [smallint] NULL,
	[UnitName] [varchar](10) NOT NULL,
	[UnitQuantity] [tinyint] NULL,
	[Price] [int] NOT NULL,
 CONSTRAINT [PK_ItemDetail] PRIMARY KEY CLUSTERED 
(
	[ItemCode] DESC,
	[billCode] DESC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[ItemInfo]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ItemInfo](
	[ItemCode] [smallint] IDENTITY(1,1) NOT NULL,
	[ItemName] [varchar](50) NOT NULL,
	[ItemUrl] [varchar](200) NOT NULL,
	[Description] [varchar](200) NOT NULL,
	[UseLimit] [varchar](200) NOT NULL,
	[catenew] [smallint] NULL,
	[cateitem] [smallint] NULL,
 CONSTRAINT [PK__ItemInfo__5EBF139D] PRIMARY KEY CLUSTERED 
(
	[ItemCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[Log]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Log](
	[Date] [datetime] NOT NULL,
	[MainType] [tinyint] NOT NULL,
	[Type] [tinyint] NOT NULL,
	[Player1] [int] NOT NULL,
	[Player2] [int] NOT NULL,
	[IID] [int] NOT NULL,
	[Value1] [int] NOT NULL,
	[Value2] [int] NOT NULL,
	[Value3] [int] NOT NULL,
	[Value4] [int] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[Login]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Login](
	[UID] [int] IDENTITY(1,1) NOT NULL,
	[ID] [varchar](16) NOT NULL,
	[PWD] [varbinary](16) NOT NULL,
	[Birth] [smalldatetime] NULL,
	[Type] [tinyint] NULL,
	[ExpTime] [int] NULL,
	[Info] [int] NULL,
	[PWD2ND] [int] NOT NULL,
	[Email] [varchar](50) NULL,
	[FirstName] [varchar](50) NULL,
	[SecondName] [varchar](50) NULL,
	[SN] [varchar](25) NULL,
	[IP] [varchar](30) NULL,
	[CreateDate] [datetime] NULL,
 CONSTRAINT [PK_Login] PRIMARY KEY CLUSTERED 
(
	[UID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY],
 CONSTRAINT [IX_Login] UNIQUE NONCLUSTERED 
(
	[ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[LoginDeleted]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[LoginDeleted](
	[UID] [int] NOT NULL,
	[ID] [varchar](16) NOT NULL,
	[DeletedTime] [smalldatetime] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[PrizeWinner]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PrizeWinner](
	[ID] [varchar](16) NOT NULL,
	[UID] [int] NOT NULL,
	[Server] [tinyint] NOT NULL,
	[Class] [tinyint] NOT NULL,
	[Quest] [smallint] NOT NULL,
 CONSTRAINT [PK_PrizeWinner] PRIMARY KEY CLUSTERED 
(
	[UID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[Rank]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Rank](
	[Server] [tinyint] NOT NULL,
	[Name] [varchar](16) NOT NULL,
	[Class] [tinyint] NOT NULL,
	[Level] [tinyint] NOT NULL,
	[Exp] [bigint] NOT NULL,
	[Guild] [varchar](16) NULL,
 CONSTRAINT [PK_Rank] PRIMARY KEY CLUSTERED 
(
	[Server] ASC,
	[Name] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
ALTER TABLE [dbo].[CLogin] ADD  CONSTRAINT [DF_CLogin_Type]  DEFAULT ((0)) FOR [Type]
GO
ALTER TABLE [dbo].[CLogin] ADD  CONSTRAINT [DF_CLogin_ExpTime]  DEFAULT ((0)) FOR [ExpTime]
GO
ALTER TABLE [dbo].[CNum] ADD  CONSTRAINT [DF_CNum_Num]  DEFAULT ((0)) FOR [Num]
GO
ALTER TABLE [dbo].[ItemBuy] ADD  CONSTRAINT [DF__ItemBuy__Discoun__77BFCB91]  DEFAULT ((0)) FOR [Discount]
GO
ALTER TABLE [dbo].[ItemBuy] ADD  CONSTRAINT [DF__ItemBuy__BuyDate__78B3EFCA]  DEFAULT (getdate()) FOR [BuyDate]
GO
ALTER TABLE [dbo].[ItemBuy] ADD  CONSTRAINT [DF__ItemBuy__Place__79A81403]  DEFAULT ((1)) FOR [Place]
GO
ALTER TABLE [dbo].[ItemBuy] ADD  CONSTRAINT [DF__ItemBuy__charDat__7A9C383C]  DEFAULT (CONVERT([varchar](10),getdate(),(126))) FOR [charDate]
GO
ALTER TABLE [dbo].[ItemBuy] ADD  CONSTRAINT [DF__ItemBuy__Flag__7B905C75]  DEFAULT ((0)) FOR [Flag]
GO
ALTER TABLE [dbo].[ItemBuy] ADD  CONSTRAINT [DF__ItemBuy__bSucces__7C8480AE]  DEFAULT ((1)) FOR [bSuccess]
GO
ALTER TABLE [dbo].[ItemBuy] ADD  CONSTRAINT [DF_ItemBuy_Age]  DEFAULT ((0)) FOR [Age]
GO
ALTER TABLE [dbo].[ItemBuy] ADD  CONSTRAINT [DF_ItemBuy_Price]  DEFAULT ((0)) FOR [Price]
GO
ALTER TABLE [dbo].[ItemDetail] ADD  CONSTRAINT [DF__ItemDetai__UnitQ__00551192]  DEFAULT ((1)) FOR [UnitQuantity]
GO
ALTER TABLE [dbo].[Login] ADD  CONSTRAINT [DF_Login_Type]  DEFAULT ((4)) FOR [Type]
GO
ALTER TABLE [dbo].[Login] ADD  CONSTRAINT [DF_Login_ExpTime]  DEFAULT ((0)) FOR [ExpTime]
GO
ALTER TABLE [dbo].[Login] ADD  DEFAULT ((0)) FOR [PWD2ND]
GO
ALTER TABLE [dbo].[Login] ADD  CONSTRAINT [DF_Login_CreateDate]  DEFAULT (getdate()) FOR [CreateDate]
GO
/****** Object:  StoredProcedure [dbo].[Block]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER OFF
GO
CREATE PROCEDURE [dbo].[Block]
	@UID int
AS
	UPDATE NMLogin SET Type = Type | 2 WHERE UID = @UID
GO
/****** Object:  StoredProcedure [dbo].[BlockCancel]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER OFF
GO
CREATE PROCEDURE [dbo].[BlockCancel]
	@UID int
AS
	UPDATE NMLogin SET Type = Type & ~2 WHERE UID = @UID
GO
/****** Object:  StoredProcedure [dbo].[ConnectedUser]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER OFF
GO

CREATE procedure  [dbo].[ConnectedUser]
	@Cnt int
as
	select count( distinct Player1) from Log
		where [Type] = 1 and
			[Date] >= convert( char(10), getdate() - @Cnt, 112) and
			[Date] < convert( char(10), getdate() - ( @Cnt - 1), 112)

GO
/****** Object:  StoredProcedure [dbo].[InsertExp]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER OFF
GO
CREATE PROCEDURE [dbo].[InsertExp]
	@Level tinyint,
	@Exp bigint
AS	
	DECLARE @ExpSum bigint

	set @ExpSum = @Exp + (select Exp from ExpTable where [Level] = @Level - 1)
	insert into ExpTable values ( @Level, @ExpSum)
GO
/****** Object:  StoredProcedure [dbo].[ItemBuyUpServer]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER OFF
GO
CREATE PROCEDURE [dbo].[ItemBuyUpServer]
	@Num int,
	@Server int
AS
	UPDATE ItemBuy SET [Server] = @Server WHERE Num = @Num AND [Flag] = 0
GO
/****** Object:  StoredProcedure [dbo].[up_ItemAmount_select]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO












CREATE          PROCEDURE [dbo].[up_ItemAmount_select]
(
	@itemCode smallint ,
	@billCode int
)
AS

/*
select Price,Unit,ItemName
from ItemDetail,ItemInfo (nolock)
where ItemDetail.ItemCode=ItemInfo.ItemCode
and billCode=@billCode and ItemDetail.ItemCode=@itemCode
*/

select Price, Unit, ItemName
from ItemDetail a (nolock)
	join ItemInfo b (nolock) on a.ItemCode=b.ItemCode
where a.billCode=@billCode and a.ItemCode=@itemCode














GO
/****** Object:  StoredProcedure [dbo].[up_itembuy_insert]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO



CREATE         procedure [dbo].[up_itembuy_insert]
(
	@Server int,
	@Userid nvarchar(12),
	@billNum char(20),
	@itemCode int,
	@billCode int,
	@quantity int,
	@unit varchar(50),
	@discount int,
	@place bit,
	@zipcode varchar(10),
	@gender bit,
	@age tinyint,
	@bSuccess bit,
	@ErrNum varchar(20),
	@price int
)
as

begin

	insert into ItemBuy (Server,Userid,billNum,ItemCode,billCode,quantity,Unit,Discount,BuyDate,Place,charDate,Zipcode,Gender,Age,bSuccess,ErrNum,Price )
	values (@Server,@Userid ,@billNum ,@itemCode ,@billCode ,@quantity ,@unit ,@discount ,getdate(),@place ,convert(varchar(10), getdate(), 126),@zipcode ,@gender ,@age ,@bSuccess,@ErrNum,@price )

end





GO
/****** Object:  StoredProcedure [dbo].[up_itembuy_Search]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER OFF
GO





CREATE             procedure [dbo].[up_itembuy_Search] 
( 
	@userid nvarchar(12)
)
as

SELECT Num,Userid,charDate,Server,ItemCode,billCode,Unit,Discount,Place,Price,quantity,giftid 
FROM  ItemBuy (nolock) 
where Userid=@userid  and bSuccess=0
order by BuyDate desc
GO
/****** Object:  StoredProcedure [dbo].[up_itembuy_select]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER OFF
GO




-- exec dbo.up_itembuy_select 'ohsillan','2004-10-01','2004-11-01'


CREATE                 procedure [dbo].[up_itembuy_select] 
( 
	@userid nvarchar(12),
	@startDate char(10),
	@endDate char(10)
)
as


SELECT Num,Userid,charDate,Server,ItemCode,billCode,Unit,Discount,Place,Price,quantity
FROM  ItemBuy (nolock)
where Userid=@userid and (giftid is null) and bSuccess=0
	and  BuyDate>=@startDate and BuyDate <= dateadd(day, 1, @endDate)
order by charDate desc
GO
/****** Object:  StoredProcedure [dbo].[up_itembuy2_Search]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER OFF
GO
CREATE              procedure [dbo].[up_itembuy2_Search] 
( 
	@userid nvarchar(12)
)
as

SELECT Num,Userid,charDate,Server,ItemCode,billCode,Unit,Discount,Place,Price,quantity,giftid 
FROM  ItemBuy (nolock) 
where Userid=@userid and (giftid is null) and bSuccess=0
order by BuyDate desc

GO
/****** Object:  StoredProcedure [dbo].[up_ItemDetail_select]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO














CREATE             PROCEDURE [dbo].[up_ItemDetail_select]
	@itemCode smallint 
AS

select billCode,ItemCode,Unit,UnitName,UnitQuantity,Price,Duration
from ItemDetail  (nolock)
where ItemCode=@itemCode
order by billCode desc















GO
/****** Object:  StoredProcedure [dbo].[up_ItemDetail_selectTop]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER OFF
GO










CREATE          PROCEDURE [dbo].[up_ItemDetail_selectTop]
	@itemCode smallint 
AS

select top 1 billCode,ItemCode,Unit,UnitName,UnitQuantity,Price,Duration
from ItemDetail  (nolock)
where ItemCode=@itemCode
order by billCode desc











GO
/****** Object:  StoredProcedure [dbo].[up_itemgift_insert]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE         procedure [dbo].[up_itemgift_insert]
(
	@Server int,
	@Userid nvarchar(12),
	@giftid nvarchar(12),
	@billNum char(20),
	@itemCode int,
	@billCode int,
	@quantity int,
	@unit varchar(50),
	@discount int,
	@place bit,
	@zipcode varchar(10),
	@gender bit,
	@age tinyint,
	@bSuccess bit,
	@ErrNum varchar(20),
	@price int,
	@msg nvarchar(50)
)
as

begin

	insert into ItemBuy(Server,Userid,giftid,billNum,ItemCode,billCode,quantity,Unit,Discount,BuyDate,Place,charDate,Zipcode,Gender,Age,bSuccess,ErrNum,Price ,msg)
	values (@Server,@Userid ,@giftid ,@billNum ,@itemCode ,@billCode ,@quantity ,@unit ,@discount ,getdate(),@place ,convert(varchar(10), getdate(), 126),@zipcode ,@gender ,@age ,@bSuccess,@ErrNum,@price,@msg )

end


GO
/****** Object:  StoredProcedure [dbo].[up_itemgift_Search]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER OFF
GO






CREATE              procedure [dbo].[up_itemgift_Search] 
( 
	@userid nvarchar(12)
)
as

SELECT Num,Userid,charDate,Server,ItemCode,billCode,Unit,Discount,Place,Price,quantity,giftid 
FROM  ItemBuy (nolock) 
where giftid=@userid  and bSuccess=0
order by BuyDate desc

GO
/****** Object:  StoredProcedure [dbo].[up_ItemInfo_select]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE [dbo].[up_ItemInfo_select]
AS

	select ItemCode,ItemName,ItemUrl,Description,UseLimit
	from ItemInfo  (nolock) 
	order by ItemCode desc 



GO
/****** Object:  StoredProcedure [dbo].[up_ItemInfo_select_cateitem]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE [dbo].[up_ItemInfo_select_cateitem]
(
	@cateitem smallint
)
AS

	select ItemCode,ItemName,ItemUrl,Description,UseLimit
	from ItemInfo  (nolock) 
	where cateitem=@cateitem
	order by ItemCode desc
GO
/****** Object:  StoredProcedure [dbo].[up_ItemInfo_select_catenew]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER ON
GO

CREATE  PROCEDURE [dbo].[up_ItemInfo_select_catenew]
(
	@catenew smallint
)
AS

	select ItemCode,ItemName,ItemUrl,Description,UseLimit
	from ItemInfo  (nolock) 
	where catenew=@catenew
	order by ItemCode desc
GO
/****** Object:  StoredProcedure [dbo].[up_iteminfo_selectName]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO


CREATE      procedure [dbo].[up_iteminfo_selectName]
(
	@itemcode smallint
)
as

select ItemName from ItemInfo  (nolock) where ItemCode=@itemcode 




GO
/****** Object:  StoredProcedure [dbo].[up_ItemInfo_selectOne]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO




CREATE        PROCEDURE [dbo].[up_ItemInfo_selectOne]
(
	@itemCode smallint 
)
AS

select ItemCode,ItemName,ItemUrl,Description,UseLimit
from ItemInfo  (nolock)
where ItemCode=@itemCode





GO
/****** Object:  StoredProcedure [dbo].[up_itemrgift_select]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE                  procedure [dbo].[up_itemrgift_select] 
( 
	@userid nvarchar(12),
	@startDate char(10),
	@endDate char(10)
)
as

SELECT Num,giftid,charDate,Server,ItemCode,billCode,Unit,Discount,Place,Price,quantity,Userid,msg
FROM  ItemBuy (nolock)
where Userid=@userid and (giftid is not  null) and  bSuccess=0
	and  BuyDate>=@startDate and BuyDate <= dateadd(day, 1, @endDate)
order by charDate desc

GO
/****** Object:  StoredProcedure [dbo].[up_itemsgift_select]    Script Date: 02.10.2018 13:52:01 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE                  procedure [dbo].[up_itemsgift_select] 
( 
	@userid nvarchar(12),
	@startDate char(10),
	@endDate char(10)
)
as

SELECT Num,Userid,charDate,Server,ItemCode,billCode,Unit,Discount,Place,Price,quantity,giftid,msg
FROM  ItemBuy (nolock)
where giftid=@userid and bSuccess=0
	and  BuyDate>=@startDate and BuyDate <= dateadd(day, 1, @endDate)
order by charDate desc

GO
USE [master]
GO
ALTER DATABASE [kal_auth] SET  READ_WRITE 
GO
