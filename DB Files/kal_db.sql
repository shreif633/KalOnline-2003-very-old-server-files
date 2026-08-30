USE [master]
GO
/****** Object:  Database [kal_db]    Script Date: 02.10.2018 13:53:21 ******/
IF NOT EXISTS (SELECT name FROM sys.databases WHERE name = N'kal_db')
BEGIN
    CREATE DATABASE [kal_db]
END
GO
ALTER DATABASE [kal_db] SET ANSI_NULL_DEFAULT OFF 
GO
ALTER DATABASE [kal_db] SET ANSI_NULLS OFF 
GO
ALTER DATABASE [kal_db] SET ANSI_PADDING OFF 
GO
ALTER DATABASE [kal_db] SET ANSI_WARNINGS OFF 
GO
ALTER DATABASE [kal_db] SET ARITHABORT OFF 
GO
ALTER DATABASE [kal_db] SET AUTO_CLOSE OFF 
GO
ALTER DATABASE [kal_db] SET AUTO_SHRINK OFF 
GO
ALTER DATABASE [kal_db] SET AUTO_UPDATE_STATISTICS ON 
GO
ALTER DATABASE [kal_db] SET CURSOR_CLOSE_ON_COMMIT OFF 
GO
ALTER DATABASE [kal_db] SET CURSOR_DEFAULT  GLOBAL 
GO
ALTER DATABASE [kal_db] SET CONCAT_NULL_YIELDS_NULL OFF 
GO
ALTER DATABASE [kal_db] SET NUMERIC_ROUNDABORT OFF 
GO
ALTER DATABASE [kal_db] SET QUOTED_IDENTIFIER OFF 
GO
ALTER DATABASE [kal_db] SET RECURSIVE_TRIGGERS OFF 
GO
ALTER DATABASE [kal_db] SET AUTO_UPDATE_STATISTICS_ASYNC OFF 
GO
ALTER DATABASE [kal_db] SET RECOVERY SIMPLE 
GO

ALTER DATABASE [kal_db] SET ANSI_NULL_DEFAULT OFF 
GO
ALTER DATABASE [kal_db] SET ANSI_NULLS OFF 
GO
ALTER DATABASE [kal_db] SET ANSI_PADDING OFF 
GO
ALTER DATABASE [kal_db] SET ANSI_WARNINGS OFF 
GO
ALTER DATABASE [kal_db] SET ARITHABORT OFF 
GO
ALTER DATABASE [kal_db] SET AUTO_CLOSE OFF 
GO
ALTER DATABASE [kal_db] SET AUTO_SHRINK OFF 
GO
ALTER DATABASE [kal_db] SET AUTO_UPDATE_STATISTICS ON 
GO
ALTER DATABASE [kal_db] SET CURSOR_CLOSE_ON_COMMIT OFF 
GO
ALTER DATABASE [kal_db] SET CURSOR_DEFAULT  GLOBAL 
GO
ALTER DATABASE [kal_db] SET CONCAT_NULL_YIELDS_NULL OFF 
GO
ALTER DATABASE [kal_db] SET NUMERIC_ROUNDABORT OFF 
GO
ALTER DATABASE [kal_db] SET QUOTED_IDENTIFIER OFF 
GO
ALTER DATABASE [kal_db] SET RECURSIVE_TRIGGERS OFF 
GO
ALTER DATABASE [kal_db] SET  DISABLE_BROKER 
GO
ALTER DATABASE [kal_db] SET AUTO_UPDATE_STATISTICS_ASYNC OFF 
GO
ALTER DATABASE [kal_db] SET DATE_CORRELATION_OPTIMIZATION OFF 
GO
ALTER DATABASE [kal_db] SET TRUSTWORTHY OFF 
GO
ALTER DATABASE [kal_db] SET ALLOW_SNAPSHOT_ISOLATION OFF 
GO
ALTER DATABASE [kal_db] SET PARAMETERIZATION SIMPLE 
GO
ALTER DATABASE [kal_db] SET READ_COMMITTED_SNAPSHOT OFF 
GO
ALTER DATABASE [kal_db] SET HONOR_BROKER_PRIORITY OFF 
GO
ALTER DATABASE [kal_db] SET RECOVERY SIMPLE 
GO
ALTER DATABASE [kal_db] SET  MULTI_USER 
GO
ALTER DATABASE [kal_db] SET PAGE_VERIFY CHECKSUM  
GO
ALTER DATABASE [kal_db] SET DB_CHAINING OFF 
GO
ALTER DATABASE [kal_db] SET FILESTREAM( NON_TRANSACTED_ACCESS = OFF ) 
GO
ALTER DATABASE [kal_db] SET TARGET_RECOVERY_TIME = 0 SECONDS 
GO
USE [kal_db]
GO
/****** Object:  User [kal]    Script Date: 02.10.2018 13:53:21 ******/
CREATE USER [kal] WITHOUT LOGIN WITH DEFAULT_SCHEMA=[kal]
GO
ALTER ROLE [db_owner] ADD MEMBER [kal]
GO
/****** Object:  Schema [kal]    Script Date: 02.10.2018 13:53:21 ******/
CREATE SCHEMA [kal]
GO
/****** Object:  Table [dbo].[BuffRemain]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[BuffRemain](
	[PID] [int] NOT NULL,
	[Type] [tinyint] NOT NULL,
	[Remain] [int] NOT NULL,
 CONSTRAINT [PK_BuffRemain] PRIMARY KEY CLUSTERED 
(
	[PID] ASC,
	[Type] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[Event]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Event](
	[PID] [int] NOT NULL,
	[Type] [tinyint] NOT NULL,
	[nF1] [int] NOT NULL,
 CONSTRAINT [PK_Event] PRIMARY KEY CLUSTERED 
(
	[PID] ASC,
	[Type] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[Friend]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Friend](
	[PID] [int] NOT NULL,
	[FPID] [int] NOT NULL,
	[FName] [varchar](14) NOT NULL,
 CONSTRAINT [PK_Friend] PRIMARY KEY NONCLUSTERED 
(
	[PID] ASC,
	[FPID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[Guild]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Guild](
	[GID] [int] NOT NULL,
	[Name] [varchar](14) NOT NULL,
	[Standard] [int] NOT NULL,
	[Exp] [char](10) NOT NULL,
	[TodayMessage] [varchar](180) NULL,
	[Leader] [varchar](14) NULL,
	[SubLeader] [varchar](14) NULL,
	[Centurion] [varchar](14) NULL,
	[Ten] [varchar](14) NULL,
	[Regular] [varchar](14) NULL,
	[Temp] [varchar](14) NULL,
	[SubLeaderAble] [int] NOT NULL,
	[CenturionAble] [int] NOT NULL,
	[TenAble] [int] NOT NULL,
	[RegularAble] [int] NOT NULL,
	[TempAble] [int] NOT NULL,
	[AID] [int] NOT NULL,
 CONSTRAINT [PK_Guild] PRIMARY KEY CLUSTERED 
(
	[GID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[GuildAlliance]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildAlliance](
	[GID1] [int] NOT NULL,
	[GID2] [int] NULL,
	[GID3] [int] NULL,
	[GID4] [int] NULL,
	[GID5] [int] NULL,
	[GID6] [int] NULL,
	[GID7] [int] NULL,
	[GID8] [int] NULL,
	[Date] [int] NOT NULL,
 CONSTRAINT [PK_GuildAlliance] PRIMARY KEY CLUSTERED 
(
	[GID1] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[GuildCastle]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildCastle](
	[CID] [int] NOT NULL,
	[GID] [int] NOT NULL,
	[Tax] [int] NOT NULL,
	[TaxRate] [tinyint] NOT NULL,
	[GateLimit] [tinyint] NOT NULL,
 CONSTRAINT [PK_GuildCastle] PRIMARY KEY CLUSTERED 
(
	[CID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[GuildMember]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildMember](
	[GID] [int] NOT NULL,
	[PID] [int] NOT NULL,
	[Class] [tinyint] NOT NULL,
	[ConnectTell] [tinyint] NOT NULL,
	[Date] [int] NOT NULL,
 CONSTRAINT [PK_GuildMember] PRIMARY KEY CLUSTERED 
(
	[PID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[GuildWar]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildWar](
	[CID] [int] NOT NULL,
	[AID] [int] NOT NULL,
	[GID] [int] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[Item]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Item](
	[PID] [int] NOT NULL,
	[IID] [int] NOT NULL,
	[Index] [smallint] NOT NULL,
	[Prefix] [tinyint] NOT NULL,
	[Info] [int] NOT NULL,
	[Num] [int] NOT NULL,
	[MaxEnd] [tinyint] NULL,
	[CurEnd] [tinyint] NULL,
	[SetGem] [tinyint] NULL,
	[XAttack] [tinyint] NULL,
	[XMagic] [tinyint] NULL,
	[XDefense] [tinyint] NULL,
	[XHit] [tinyint] NULL,
	[XDodge] [tinyint] NULL,
	[Protect] [tinyint] NULL,
	[UpgrLevel] [tinyint] NULL,
	[UpgrRate] [tinyint] NULL,
	[PetTime] [int] NOT NULL,
	[Lock] [varchar](16) NOT NULL,
	[ItemStat] [int] NOT NULL,
 CONSTRAINT [PK_Item] PRIMARY KEY CLUSTERED 
(
	[IID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[ItemRestored]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ItemRestored](
	[Date] [smalldatetime] NOT NULL,
	[PID] [int] NOT NULL,
	[IID] [int] NOT NULL,
	[Index] [smallint] NOT NULL,
	[Num] [int] NOT NULL,
	[Val1] [int] NOT NULL,
	[Val2] [int] NOT NULL,
	[Val3] [int] NOT NULL,
	[PayBack] [int] NOT NULL,
	[Remark] [varchar](48) NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[Log]    Script Date: 02.10.2018 13:53:21 ******/
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
/****** Object:  Table [dbo].[Mail]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Mail](
	[MID] [int] IDENTITY(1,1) NOT NULL,
	[Priority] [tinyint] NOT NULL,
	[Type] [tinyint] NOT NULL,
	[Status] [tinyint] NOT NULL,
	[SPID] [int] NOT NULL,
	[SName] [varchar](14) NOT NULL,
	[RPID] [int] NOT NULL,
	[RName] [varchar](14) NOT NULL,
	[TimetS] [int] NOT NULL,
	[TimetE] [int] NOT NULL,
	[IID] [int] NOT NULL,
	[Ver] [tinyint] NOT NULL,
	[Index] [smallint] NOT NULL,
	[Prefix] [tinyint] NOT NULL,
	[Num] [int] NOT NULL,
	[Pay] [int] NOT NULL,
	[Msg] [varbinary](512) NOT NULL,
 CONSTRAINT [PK_Mail] PRIMARY KEY CLUSTERED 
(
	[MID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[MLM]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[MLM](
	[PID] [int] NOT NULL,
	[Relation] [tinyint] NOT NULL,
	[Name] [varchar](14) NOT NULL,
	[RPID] [int] NOT NULL,
	[RUID] [int] NOT NULL,
	[Class] [tinyint] NOT NULL,
	[Level] [tinyint] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[MLMMsg]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[MLMMsg](
	[PID] [int] NOT NULL,
	[Msg] [varchar](180) NOT NULL,
 CONSTRAINT [PK_MLMMsg] PRIMARY KEY CLUSTERED 
(
	[PID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[NameChanged]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[NameChanged](
	[Date] [smalldatetime] NOT NULL,
	[Type] [tinyint] NOT NULL,
	[PID] [int] NOT NULL,
	[NameOld] [varchar](14) NOT NULL,
	[NameNew] [varchar](14) NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[PKBulletin]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PKBulletin](
	[Index] [int] IDENTITY(1,1) NOT NULL,
	[NameGuild] [varchar](14) NOT NULL,
	[NameCatch] [varchar](14) NOT NULL,
	[NameAssassin] [varchar](14) NOT NULL,
 CONSTRAINT [PK_PKBulletin] PRIMARY KEY CLUSTERED 
(
	[Index] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[Player]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Player](
	[UID] [int] NOT NULL,
	[PID] [int] IDENTITY(1,1) NOT NULL,
	[Admin] [tinyint] NOT NULL,
	[Name] [varchar](14) NOT NULL,
	[Class] [tinyint] NOT NULL,
	[Specialty] [tinyint] NOT NULL,
	[Level] [tinyint] NOT NULL,
	[Contribute] [smallint] NOT NULL,
	[Exp] [bigint] NOT NULL,
	[GID] [int] NOT NULL,
	[GRole] [tinyint] NOT NULL,
	[Strength] [tinyint] NOT NULL,
	[Health] [tinyint] NOT NULL,
	[Intelligence] [tinyint] NOT NULL,
	[Wisdom] [tinyint] NOT NULL,
	[Dexterity] [tinyint] NOT NULL,
	[CurHP] [smallint] NOT NULL,
	[CurMP] [smallint] NOT NULL,
	[PUPoint] [smallint] NOT NULL,
	[SUPoint] [smallint] NOT NULL,
	[Killed] [tinyint] NOT NULL,
	[Map] [tinyint] NOT NULL,
	[X] [int] NOT NULL,
	[Y] [int] NOT NULL,
	[Z] [int] NOT NULL,
	[Face] [tinyint] NOT NULL,
	[Hair] [tinyint] NOT NULL,
	[RevivalId] [tinyint] NOT NULL,
	[Rage] [int] NOT NULL,
	[HonorPoint] [int] NOT NULL,
	[HonorKill] [int] NOT NULL,
	[HonorDeath] [int] NOT NULL,
	[DKPTotal] [int] NOT NULL,
	[DKPWin] [int] NOT NULL,
	[PLTotal] [int] NOT NULL,
	[PLWin] [int] NOT NULL,
	[SVTotal] [int] NOT NULL,
	[SVWin] [int] NOT NULL,
	[RewardPoint] [int] NOT NULL,
	[EmokDay] [int] NOT NULL,
	[EmokTime] [int] NOT NULL,
 CONSTRAINT [PK_Player] PRIMARY KEY CLUSTERED 
(
	[PID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY],
 CONSTRAINT [IX_Player_1] UNIQUE NONCLUSTERED 
(
	[Name] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[PlayerDeleted]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PlayerDeleted](
	[PID] [int] NOT NULL,
	[UID] [int] NOT NULL,
	[DeletedTime] [smalldatetime] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[Quest]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Quest](
	[PID] [int] NOT NULL,
	[Quest] [smallint] NOT NULL,
	[Flag] [tinyint] NOT NULL,
	[Clear] [tinyint] NOT NULL,
	[QuestTime] [int] NOT NULL,
	[QuestRepeat] [int] NOT NULL,
	[MonsterCount] [int] NOT NULL,
 CONSTRAINT [PK_Quest] PRIMARY KEY NONCLUSTERED 
(
	[PID] ASC,
	[Quest] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[ReservedName]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ReservedName](
	[Name] [varchar](14) NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[Shortcut]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Shortcut](
	[PID] [int] NOT NULL,
	[Keys] [binary](40) NOT NULL,
 CONSTRAINT [PK_Shortcut] PRIMARY KEY CLUSTERED 
(
	[PID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[Skill]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Skill](
	[PID] [int] NOT NULL,
	[Index] [tinyint] NOT NULL,
	[Level] [tinyint] NOT NULL,
 CONSTRAINT [PK_Skill] PRIMARY KEY NONCLUSTERED 
(
	[PID] ASC,
	[Index] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[Statistics]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Statistics](
	[Date] [smalldatetime] NOT NULL,
	[Knight] [smallint] NOT NULL,
	[Mage] [smallint] NOT NULL,
	[Archer] [smallint] NOT NULL,
	[TotalUser] [smallint] NOT NULL,
	[FishTrap] [smallint] NOT NULL,
	[PrivateShop] [smallint] NOT NULL,
	[Currency] [int] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[Teleport]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Teleport](
	[PID] [int] NOT NULL,
	[LocName] [varchar](20) NOT NULL,
	[Map] [tinyint] NOT NULL,
	[X] [int] NOT NULL,
	[Y] [int] NOT NULL,
	[Z] [int] NOT NULL,
 CONSTRAINT [PK_Teleport] PRIMARY KEY CLUSTERED 
(
	[PID] ASC,
	[LocName] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
ALTER TABLE [dbo].[Event] ADD  CONSTRAINT [DF_Event_byF1]  DEFAULT ((0)) FOR [nF1]
GO
ALTER TABLE [dbo].[Guild] ADD  CONSTRAINT [DF_Guild_Standard]  DEFAULT ((0)) FOR [Standard]
GO
ALTER TABLE [dbo].[Guild] ADD  CONSTRAINT [DF_Guild_Exp]  DEFAULT ((0)) FOR [Exp]
GO
ALTER TABLE [dbo].[Guild] ADD  CONSTRAINT [DF_Guild_SubLeaderAble]  DEFAULT ((0)) FOR [SubLeaderAble]
GO
ALTER TABLE [dbo].[Guild] ADD  CONSTRAINT [DF_Guild_CenturionAble]  DEFAULT ((0)) FOR [CenturionAble]
GO
ALTER TABLE [dbo].[Guild] ADD  CONSTRAINT [DF_Guild_TenAble]  DEFAULT ((0)) FOR [TenAble]
GO
ALTER TABLE [dbo].[Guild] ADD  CONSTRAINT [DF_Guild_RegularAble]  DEFAULT ((0)) FOR [RegularAble]
GO
ALTER TABLE [dbo].[Guild] ADD  CONSTRAINT [DF_Guild_TempAble]  DEFAULT ((0)) FOR [TempAble]
GO
ALTER TABLE [dbo].[Guild] ADD  CONSTRAINT [DF_Guild_AID]  DEFAULT ((0)) FOR [AID]
GO
ALTER TABLE [dbo].[GuildCastle] ADD  CONSTRAINT [DF_GuildCastle_GID]  DEFAULT ((0)) FOR [GID]
GO
ALTER TABLE [dbo].[GuildCastle] ADD  CONSTRAINT [DF_GuildCastle_Tax]  DEFAULT ((0)) FOR [Tax]
GO
ALTER TABLE [dbo].[GuildCastle] ADD  CONSTRAINT [DF_GuildCastle_TaxRate]  DEFAULT ((100)) FOR [TaxRate]
GO
ALTER TABLE [dbo].[GuildCastle] ADD  CONSTRAINT [DF_GuildCastle_GateLimit]  DEFAULT ((0)) FOR [GateLimit]
GO
ALTER TABLE [dbo].[GuildMember] ADD  CONSTRAINT [DF_GuildMember_Date]  DEFAULT ((0)) FOR [Date]
GO
ALTER TABLE [dbo].[Item] ADD  CONSTRAINT [DF_Item_MaxEnd]  DEFAULT ((0)) FOR [MaxEnd]
GO
ALTER TABLE [dbo].[Item] ADD  CONSTRAINT [DF_Item_CurEnd]  DEFAULT ((0)) FOR [CurEnd]
GO
ALTER TABLE [dbo].[Item] ADD  CONSTRAINT [DF_Item_SetGem]  DEFAULT ((0)) FOR [SetGem]
GO
ALTER TABLE [dbo].[Item] ADD  CONSTRAINT [DF_Item_XAttack]  DEFAULT ((0)) FOR [XAttack]
GO
ALTER TABLE [dbo].[Item] ADD  CONSTRAINT [DF_Item_XMagic]  DEFAULT ((0)) FOR [XMagic]
GO
ALTER TABLE [dbo].[Item] ADD  CONSTRAINT [DF_Item_XDefense]  DEFAULT ((0)) FOR [XDefense]
GO
ALTER TABLE [dbo].[Item] ADD  CONSTRAINT [DF_Item_XHit]  DEFAULT ((0)) FOR [XHit]
GO
ALTER TABLE [dbo].[Item] ADD  CONSTRAINT [DF_Item_XDodge]  DEFAULT ((0)) FOR [XDodge]
GO
ALTER TABLE [dbo].[Item] ADD  CONSTRAINT [DF_Item_Protect]  DEFAULT ((0)) FOR [Protect]
GO
ALTER TABLE [dbo].[Item] ADD  CONSTRAINT [DF_Item_UpgrLevel]  DEFAULT ((0)) FOR [UpgrLevel]
GO
ALTER TABLE [dbo].[Item] ADD  CONSTRAINT [DF_Item_UpgrRate]  DEFAULT ((0)) FOR [UpgrRate]
GO
ALTER TABLE [dbo].[Item] ADD  DEFAULT ((0)) FOR [PetTime]
GO
ALTER TABLE [dbo].[Item] ADD  DEFAULT ('nopwd') FOR [Lock]
GO
ALTER TABLE [dbo].[Item] ADD  DEFAULT ((0)) FOR [ItemStat]
GO
ALTER TABLE [dbo].[Player] ADD  CONSTRAINT [DF_Player_Admin]  DEFAULT ((0)) FOR [Admin]
GO
ALTER TABLE [dbo].[Player] ADD  CONSTRAINT [DF_Player_Specialty]  DEFAULT ((1)) FOR [Specialty]
GO
ALTER TABLE [dbo].[Player] ADD  CONSTRAINT [DF_Player_Level]  DEFAULT ((1)) FOR [Level]
GO
ALTER TABLE [dbo].[Player] ADD  CONSTRAINT [DF_Player_Reputation]  DEFAULT ((0)) FOR [Contribute]
GO
ALTER TABLE [dbo].[Player] ADD  CONSTRAINT [DF_Player_Exp]  DEFAULT ((0)) FOR [Exp]
GO
ALTER TABLE [dbo].[Player] ADD  CONSTRAINT [DF_Player_GID]  DEFAULT ((0)) FOR [GID]
GO
ALTER TABLE [dbo].[Player] ADD  CONSTRAINT [DF_Player_GRole]  DEFAULT ((0)) FOR [GRole]
GO
ALTER TABLE [dbo].[Player] ADD  CONSTRAINT [DF_Player_PUPoint]  DEFAULT ((0)) FOR [PUPoint]
GO
ALTER TABLE [dbo].[Player] ADD  CONSTRAINT [DF_Player_SUPoint]  DEFAULT ((0)) FOR [SUPoint]
GO
ALTER TABLE [dbo].[Player] ADD  CONSTRAINT [DF_Player_Killed]  DEFAULT ((0)) FOR [Killed]
GO
ALTER TABLE [dbo].[Player] ADD  CONSTRAINT [DF_Player_RevivalId]  DEFAULT ((0)) FOR [RevivalId]
GO
ALTER TABLE [dbo].[Player] ADD  CONSTRAINT [DF_Player_Rage]  DEFAULT ((0)) FOR [Rage]
GO
ALTER TABLE [dbo].[Player] ADD  DEFAULT ((0)) FOR [HonorPoint]
GO
ALTER TABLE [dbo].[Player] ADD  DEFAULT ((0)) FOR [HonorKill]
GO
ALTER TABLE [dbo].[Player] ADD  DEFAULT ((0)) FOR [HonorDeath]
GO
ALTER TABLE [dbo].[Player] ADD  DEFAULT ((0)) FOR [DKPTotal]
GO
ALTER TABLE [dbo].[Player] ADD  DEFAULT ((0)) FOR [DKPWin]
GO
ALTER TABLE [dbo].[Player] ADD  DEFAULT ((0)) FOR [PLTotal]
GO
ALTER TABLE [dbo].[Player] ADD  DEFAULT ((0)) FOR [PLWin]
GO
ALTER TABLE [dbo].[Player] ADD  DEFAULT ((0)) FOR [SVTotal]
GO
ALTER TABLE [dbo].[Player] ADD  DEFAULT ((0)) FOR [SVWin]
GO
ALTER TABLE [dbo].[Player] ADD  DEFAULT ((0)) FOR [RewardPoint]
GO
ALTER TABLE [dbo].[Player] ADD  DEFAULT ((0)) FOR [EmokDay]
GO
ALTER TABLE [dbo].[Player] ADD  DEFAULT ((0)) FOR [EmokTime]
GO
ALTER TABLE [dbo].[Quest] ADD  DEFAULT ((0)) FOR [QuestTime]
GO
ALTER TABLE [dbo].[Quest] ADD  DEFAULT ((0)) FOR [QuestRepeat]
GO
ALTER TABLE [dbo].[Quest] ADD  DEFAULT ((0)) FOR [MonsterCount]
GO
ALTER TABLE [dbo].[Statistics] ADD  CONSTRAINT [DF_Statistics_Date]  DEFAULT (getdate()) FOR [Date]
GO
/****** Object:  StoredProcedure [dbo].[CreatePlayer]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE [dbo].[CreatePlayer]

@uid int,
@name varchar(14),
@class tinyint,
@strength tinyint,
@health tinyint,
@intelligence tinyint,
@wisdom tinyint,
@dexterity tinyint,
@curhp smallint,
@curmp smallint,
@map tinyint,
@x int,
@y int,
@z int,
@face tinyint,
@hair tinyint

AS

DECLARE @PID int
DECLARE @GM int
DECLARE @globalgm int
DECLARE @itemid int
DECLARE @startitems int
DECLARE @knightx int
DECLARE @knighty int
DECLARE @knightz int
DECLARE @knightm int
DECLARE @magex int
DECLARE @magey int
DECLARE @magez int
DECLARE @magem int
DECLARE @archerx int
DECLARE @archery int
DECLARE @archerz int
DECLARE @archerm int
DECLARE @thiefx int
DECLARE @thiefy int
DECLARE @thiefz int
DECLARE @thiefm int


SET @GM = 0

---- Create the variables for the random number generation
DECLARE @Randomx INT;
DECLARE @Upperx INT;
DECLARE @Lowerx INT

SET @Lowerx = 257285 ---- The lowest random number
SET @Upperx = 257351 ---- The highest random number

SELECT @Randomx = ROUND(((@Upperx - @Lowerx -1) * RAND() + @Lowerx), 0)
SELECT @Randomx


DECLARE @Randomy INT;
DECLARE @Uppery INT;
DECLARE @Lowery INT

SET @Lowery = 258774 ---- The lowest random number
SET @Uppery = 258744 ---- The highest random number

SELECT @Randomy = ROUND(((@Uppery - @Lowery -1) * RAND() + @Lowery), 0)
SELECT @Randomy




--***************************************
SET @startitems = 0 --set to 0 to disable
--Knight*********************************
SET @knightx = @Randomx -- X Coord
SET @knighty = @Randomy -- Y Coord
SET @knightz = 17320 -- Z Coord
SET @knightm = 0 -- MAP
--Mage***********************************
SET @magex = @Randomx -- X Coord
SET @magey = @Randomy -- Y Coord
SET @magez = 17320 -- Z Coord
SET @magem = 0 -- MAP
--Archer*********************************
SET @archerx = @Randomx -- X Coord
SET @archery = @Randomy -- Y Coord
SET @archerz = 17320 -- Z Coord
SET @archerm = 0 -- MAP
--Thief*********************************
SET @thiefx = @Randomx -- X Coord
SET @thiefy = @Randomy -- Y Coord
SET @thiefz = 17320 -- Z Coord
SET @thiefm = 0 -- MAP
--***************************************


--Start Koordinates für jedes Klasse extra zuweisen
IF (@Class = 0) --knight
begin
Set @x = @knightx
Set @y = @knighty
Set @z = @knightz
Set @map = @knightm
end
ELSE IF (@Class = 1) --mage
begin
Set @x = @magex
Set @y = @magey
Set @z = @magez
Set @map = @magem
end
ELSE IF (@Class = 2) --archer
begin
Set @x = @archerx
Set @y = @archery
Set @z = @archerz
Set @map = @archerm
end
ELSE IF (@Class = 3) --thief
begin
Set @x = @thiefx
Set @y = @thiefy
Set @z = @thiefz
Set @map = @thiefm
end



SET @x = @Randomx
SET @y = @Randomy
SET @z = 15964

DECLARE @ranexp INT;
DECLARE @expup INT;
DECLARE @explow INT

SET @explow = 0 ---- The lowest random number
SET @expup = 12320243 ---- The highest random number

SELECT @ranexp = ROUND(((@expup - @explow -1) * RAND() + @explow), 0)
SELECT @ranexp



INSERT INTO Player ( [UID], [Admin], [Name], [Class], [Strength], [Health], [Intelligence], [Wisdom], [Dexterity], [CurHP], [CurMP], [Map], [X], [Y], [Z], [Face], [Hair]) VALUES ( @UID, @GM, @Name, @Class, @Strength, @Health, @Intelligence, @Wisdom, @Dexterity, @CurHP, @CurMP, @map, @x, @y, @z, @Face, @Hair)


--SET @PID = (select max([PID]) from Player WHERE [UID] = @UID)
--INSERT INTO Quest ( [PID], [Quest], [Flag], [Clear]) VALUES (@PID, 9001, 1, 0)
GO
/****** Object:  StoredProcedure [dbo].[Insert_Item]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER OFF
GO
CREATE procedure [dbo].[Insert_Item]
	@UID int
as

	declare @PID int
	declare @IID int
	declare @Job tinyint
	select @IID = max( IID) from Item

	declare Player_Cursor cursor for
	select [PID] from Player where [UID] = @UID
	open Player_Cursor

	fetch next from Player_Cursor into @PID
	while (@@FETCH_STATUS =0)
	begin
		select @Job = [Class] from Player where [PID] = @PID
		if ( @Job = 0)
		begin
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 1, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 2, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 3, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 4, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 5, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 6, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 7, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 8, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 9, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 10, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 11, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 12, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 13, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 14, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 15, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 16, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 17, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 18, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 19, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 20, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 21, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 28, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 29, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 30, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 49, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 50, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 51, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 52, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 53, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 59, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 60, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 61, 0, 8, 1, 100, 100, 0) 
		end -- if end
		else if ( @Job = 2)
		begin
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 22, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 23, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 24, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 25, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 26, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 27, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 32, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 33, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 34, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 35, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 36, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 37, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 38, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 39, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 40, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 41, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 42, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 43, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 44, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 45, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 46, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 54, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 55, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 56, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 57, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 58, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 62, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 63, 0, 8, 1, 100, 100, 0) 
			set @IID = @IID + 1
			insert into Item (PID, IID, [Index], Prefix, Info, Num, MaxEnd, CurEnd, SetGem) values ( @PID, @IID, 64, 0, 8, 1, 100, 100, 0) 
		end -- else if end

		fetch next from Player_Cursor into @PID
	end -- while end

	close Player_Cursor
	deallocate Player_Cursor
GO
/****** Object:  StoredProcedure [dbo].[Insert_RsvdPlayer]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER OFF
GO
CREATE procedure [dbo].[Insert_RsvdPlayer]
	@Name varchar(14)
as
	insert into Player ( UID, Name, Class, Strength, Health, Intelligence, Wisdom, Dexterity, CurHP, CurMP, Map, X, Y, Z, Face, Hair)
			VALUES ( 0, @Name, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
GO
/****** Object:  StoredProcedure [dbo].[NameChange]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER ON
GO

CREATE PROCEDURE [dbo].[NameChange]
	@NameOld varchar(14),
	@NameNew varchar(14)
AS		
	DECLARE @PID int
	SELECT @PID = [PID] FROM Player WHERE [Name] = @NameOld
	if (select count(*) from Player where [Name] = @NameOld) > 0
	begin
		 if ( select count(*) from Player where [Name] = @NameNew) = 0
		begin		
			update Player set [Name] = @NameNew where [Name] = @NameOld
			update MLM set [Name] = @NameNew where [Name] = @NameOld
			update Friend set [FName] = @NameNew where [FName] = @NameOld
			update Mail set [SName] = @NameNew where [SName] = @NameOld
			update Mail set [RName] = @NameNew where [RName] = @NameOld
			if @@ERROR = 0 
			begin  
				--INSERT INTO NameChanged VALUES ( GETDATE(), @PID, @NameOld, @NameNew)
				PRINT '성공적으로 이름이  바뀌었습니다.'
			end
			else
				PRINT '실패하였습니다. '  				
		end
		else
			PRINT '새로운 이름이 이미 존재 합니다.'			
	end
	else
		PRINT '기존이름이 없습니다.'
GO
/****** Object:  StoredProcedure [dbo].[RestoreItem]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER ON
GO

CREATE PROCEDURE [dbo].[RestoreItem]
	@MainType int,
	@DelType int,
	@PID int,
	@Val3 int,
	@IID int,
	@Index smallint,
	@Num int,
	@Val1 int,
	@Val2 int,
	@PayBack int
AS
	IF ( @MainType = 1)
	BEGIN
		DECLARE @Info tinyint
		DECLARE @Prefix tinyint
		DECLARE @MaxEnd tinyint
		DECLARE @SetGem tinyint
		DECLARE @XAttack tinyint
		DECLARE @XMagic tinyint
		DECLARE @XDefense tinyint
		DECLARE @XHit tinyint
		DECLARE @XDodge tinyint
		DECLARE @Protect tinyint
		DECLARE @UpgrLevel tinyint
		DECLARE @UpgrRate tinyint

		SET @Info = 0
		IF( @Index = 337 or @Index = 338 or @Index = 339)
			SET @Info  = 32
		SET @Prefix = ( @Val1 & 0xff000000) / 0x1000000
		SET @MaxEnd = ( @Val1 & 0xff0000) / 0x10000
		SET @SetGem = ( @Val1 & 0xff00) / 0x100
		SET @XAttack = ( @Val1 & 0xff)
		SET @XMagic = ( @Val2 & 0xff000000) / 0x1000000
		SET @XDefense = ( @Val2 & 0xff0000) / 0x10000
		SET @XHit = ( @Val2 & 0xff00) / 0x100
		SET @XDodge = ( @Val2 & 0xff)
		SET @Protect = ( @Val3 & 0xff000000) / 0x1000000
		SET @UpgrLevel = ( @Val3 & 0xff0000) / 0x10000
		SET @UpgrRate = ( @Val3 & 0xff00) / 0x100
	
		IF ( @DelType = 8)
		BEGIN
			IF ( @PayBack > 0)
			BEGIN
				DECLARE @MoneyIID int
				DECLARE @CurMoney int
				SELECT @MoneyIID = [IID], @CurMoney = [Num] FROM Item WHERE [PID] = @PID AND [Index] = 31 AND ( [Info] & 16) = 0
				IF( @CurMoney > @PayBack)
				BEGIN
					UPDATE Item SET [Num] = [Num] - @PayBack WHERE [IID] = @MoneyIID
					INSERT INTO Item VALUES ( @PID, @IID, @Index, @Prefix, @Info, @Num, @MaxEnd, 1, @SetGem, @XAttack, @XMagic, @XDefense, @XHit, @XDodge, @Protect, @UpgrLevel, @UpgrRate, 0, 0, 0)
					INSERT INTO ItemRestored VALUES ( GETDATE(), @PID, @IID, @Index, @Num, @Val1, @Val2, @Val3, @PayBack, '')
					PRINT '성공적으로 복구되었습니다.'
				END
				ELSE IF ( @CurMoney = @PayBack)
				BEGIN
					DELETE FROM Item WHERE [IID] = @MoneyIID
					INSERT INTO Item VALUES ( @PID, @IID, @Index, @Prefix, @Info, @Num, @MaxEnd, 1, @SetGem, @XAttack, @XMagic, @XDefense, @XHit, @XDodge, @Protect, @UpgrLevel, @UpgrRate, 0, 0, 0)
					INSERT INTO ItemRestored VALUES ( GETDATE(), @PID, @IID, @Index, @Num, @Val1, @Val2, @Val3, @PayBack, 'MD(IID:' + CAST( @MoneyIID as varchar(20)) + ')')
					PRINT '성공적으로 복구되었습니다.'
				END
				ELSE
					PRINT '회수될 금액이 부족하여 복구가 취소되었습니다.'
			END
			ELSE -- if paypack
				PRINT '회수할 금액이 유효하지 않아 복구가 취소되었습니다.'
		END
		ELSE -- if deltype
		BEGIN
			INSERT INTO Item VALUES ( @PID, @IID, @Index, @Prefix, @Info, @Num, @MaxEnd, 1, @SetGem, @XAttack, @XMagic, @XDefense, @XHit, @XDodge, @Protect, @UpgrLevel, @UpgrRate, 0, 0, 0)
			INSERT INTO ItemRestored VALUES ( GETDATE(), @PID, @IID, @Index, @Num, @Val1, @Val2, @Val3, 0, '')
			PRINT '성공적으로 복구되었습니다.'
		END
	END
	ELSE	-- MainType
		PRINT '삭제된 LOG 기록이 아닙니다!!!'
GO
/****** Object:  StoredProcedure [dbo].[RestorePlayer]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE [dbo].[RestorePlayer]
	@Name varchar( 14)
AS
	DECLARE @PID int
	SELECT @PID = [PID] FROM Player WHERE [Name] = @Name
	IF @@ROWCOUNT > 0
	BEGIN
		DECLARE @UID int
		SELECT @UID = [UID] FROM PlayerDeleted WHERE [PID] = @PID
		IF @@ROWCOUNT > 0
		BEGIN
			IF ( SELECT COUNT(*) FROM Player WHERE [UID] = @UID) < 3
			BEGIN
				UPDATE Player SET [UID] = @UID WHERE [PID] = @PID
				DELETE FROM PlayerDeleted WHERE PID = @PID
				PRINT '정상적으로 복구되었습니다'
			END
			ELSE
				PRINT '이미 3개의 캐릭터가 존재하여 복구할 수 없습니다.'
		END
		ELSE	-- @UID
			PRINT '복구에 문제가 발생하였습니다. 담당자에게 문의바랍니다'
	END
	ELSE	-- @PID
		PRINT '존재하지 않거나 저장기간(2주)이 초과되어 완전삭제된 캐릭터입니다.'

GO
/****** Object:  StoredProcedure [dbo].[SkillRedistribute]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER OFF
GO

CREATE PROCEDURE [dbo].[SkillRedistribute]
	@Name varchar(15)
AS	
	DECLARE @PID int
	DECLARE @Class tinyint
	DECLARE @Level tinyint
	DECLARE @Specialty tinyint


	declare Player_Cursor cursor for
	select [PID], [Class], [Level], [Specialty] from Player where [Name] = @Name 
	open Player_Cursor

	fetch next from Player_Cursor into @PID, @Class, @Level, @Specialty
	if (@@FETCH_STATUS = 0)
	begin
		delete from Skill where [PID] = @PID

		insert into Skill values ( @PID, 0, 1) -- 전력질주
		insert into Skill values ( @PID, 1, 1) -- 참수

		if( @Class = 0)
		begin
			if ( @Level >= 15)
				insert into Skill values ( @PID, 14, 1) -- 보호하기
			if ( @Level >= 18)
				insert into Skill values ( @PID, 4, 1) -- 우롱하기
			if ( @Level >= 25)
				insert into Skill values ( @PID, 15, 1) -- 방패치기
			if ( @Specialty = 3)
				insert into Skill values ( @PID, 6, 1) -- 막아내기

			if( @Specialty = 11) --장군
			begin
				if( @Level < 56) -- 방어의기운
					insert into Skill values ( @PID, 18, 1)
				else if( @Level < 62)
					insert into Skill values ( @PID, 18, 2)
				else if( @Level < 70)
					insert into Skill values ( @PID, 18, 3)
				else
					insert into Skill values ( @PID, 18, 4)

				if( @Level >= 66) --보호하기
					update Skill set [Level] = 3 where PID = @PID and [Index] = 14
				else if( @Level >= 53)
					update Skill set [Level] = 2 where PID = @PID and [Index] = 14
			end
		end
		else if( @Class = 1)
		begin
			insert into Skill values ( @PID, 4, 1) -- 낙뢰술
			if ( @Level >= 25)
				insert into Skill values ( @PID, 5, 1) -- 원거리보호
			if ( @Specialty = 3)
			begin
				insert into Skill values ( @PID, 17, 1) -- 인염술
				if( @Level >= 35)
					insert into Skill values ( @PID, 18, 1) -- 인해술
				if( @Level >= 40)
					insert into Skill values ( @PID, 19, 1) -- 인노술
				if( @Level >= 45)
					insert into Skill values ( @PID, 20, 1) -- 인거술
				if( @Level >= 50)
					insert into Skill values ( @PID, 21, 1) -- 인화술
			end
		end
		else if( @Class = 2)
		begin
			if ( @Level >= 25)
				insert into Skill values ( @PID, 15, 1) -- 침묵의화살	
			if ( @Specialty = 3 and @Level >= 30)			
				insert into Skill values ( @PID, 7, (@Level - 30) / 5 + 1 ) -- 뛰어난기회포착
		end

		-- 1차전직 기술 습득
		if ( @Specialty >= 3)
			insert into Skill values ( @PID, 11, 2) -- 휴식2레벨
		else
			insert into Skill values ( @PID, 11, 1) -- 휴식1레벨

		-- 2차전직 기술 습득
		if ( @Specialty >= 7)
			insert into Skill values ( @PID, 30, (@Level - 45) / 5) -- 생명력강화
	END	
	close Player_Cursor
	deallocate Player_Cursor

	update Player set SUPoint  = [Level] - 1 + (select  count(*)  from  Quest where  Quest.PID = @PID
	 and Quest.[Quest] in ( 7, 11, 12, 13, 14, 16, 19, 21, 22, 23, 24, 33)
	 and Quest.[Clear] = 1) where [PID] = @PID
GO
/****** Object:  StoredProcedure [dbo].[USP_040227]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER OFF
GO

CREATE procedure [dbo].[USP_040227]
as
/*
	1. 스킬초기화
	2. 스킬포인트 재분배
	3. 공헌도 재분배
*/


	declare @PID int
	declare @Class tinyint

--	1. 스킬초기화
	delete from Skill

	declare Player_Cursor cursor for
	select [PID] from Player
	open Player_Cursor

	fetch next from Player_Cursor into @PID
	while (@@FETCH_STATUS = 0)
	begin
		select @Class = [Class] from Player where [PID] = @PID

		insert into Skill values ( @PID, 0, 1) -- boost
		insert into Skill values ( @PID, 1, 1) -- behead
		if ( @Class = 1)
			insert into Skill values ( @PID, 4, 1) -- litning

		fetch next from Player_Cursor into @PID
	end -- while end

	close Player_Cursor
	deallocate Player_Cursor


--	2. 스킬포인트 재분배
	update Player set SUPoint = [Level] - 1
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 7 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 11 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 12 and b.Clear = 1)
--	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 13 and b.Clear = 1)
--	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 14 and b.Clear = 1)


--	3. 공헌도 재분배
	update Player set Contribute = 0
	update Player set Contribute = a.Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 3 and b.Clear = 1)
	update Player set Contribute = a.Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 4 and b.Flag = 4 and b.Clear = 1)
	update Player set Contribute = a.Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 5 and b.Clear = 1)
	update Player set Contribute = a.Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 6 and b.Clear = 1)
	update Player set Contribute = a.Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 8 and b.Clear = 1)
	update Player set Contribute = a.Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 10 and b.Clear = 1)
	update Player set Contribute = a.Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 12 and b.Flag = 3 and b.Clear = 1)
--	update Player set Contribute = a.Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 14 and b.Flag = 7 and b.Clear = 1)

GO
/****** Object:  StoredProcedure [dbo].[USP_040312]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER ON
GO

CREATE PROCEDURE [dbo].[USP_040312]
AS
/*
	1. Player 테이블에서 NetHP / NetMP 칼럼 삭제
	2. 모든 캐릭터 스탯초기화
*/


--	1. Player 테이블에서 NetHP / NetMP 칼럼 삭제
ALTER TABLE dbo.Player ADD CONSTRAINT
	DF_Player_NetHP DEFAULT 0 FOR NetHP

ALTER TABLE dbo.Player ADD CONSTRAINT
	DF_Player_NetMP DEFAULT 0 FOR NetMP


--	2. 모든 캐릭터 스탯초기화
UPDATE Player SET [CurHP] = 52 * [Level] / 3 + 166, [CurMP] = 8 * [Level] + 157, [Strength] = 18, [Health] = 16, [Intelligence] = 8, [Wisdom] = 8, [Dexterity] = 10, [PUPoint] = [Level] * 5 WHERE [Class] = 0 -- 무사
UPDATE Player SET [CurHP] = 52 * [Level] / 3 + 129, [CurMP] = 8 * [Level] + 207, [Strength] = 8, [Health] = 10, [Intelligence] = 18, [Wisdom] = 16, [Dexterity] = 8, [PUPoint] = [Level] * 5 WHERE [Class] = 1 -- 주술사
UPDATE Player SET [CurHP] = 52 * [Level] / 3 + 130, [CurMP] = 8 * [Level] + 166, [Strength] = 14, [Health] = 10, [Intelligence] = 8, [Wisdom] = 10, [Dexterity] = 18, [PUPoint] = [Level] * 5 WHERE [Class] = 2 -- 궁수

GO
/****** Object:  StoredProcedure [dbo].[USP_040325]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER OFF
GO

CREATE procedure [dbo].[USP_040325]
as
/*
	1. 스킬초기화
	2. 스킬포인트 재분배
	3. 26번 퀘스트 DB에서 삭제
*/


--	1. 스킬초기화
	delete from Skill

	declare @PID int
	declare @Class tinyint
	declare @Specialty tinyint

	-- reset skills..
	declare Player_Cursor cursor for
	select [PID], [Class], [Specialty] from Player
	open Player_Cursor

	fetch next from Player_Cursor into @PID, @Class, @Specialty
	while (@@FETCH_STATUS = 0)
	begin
		insert into Skill values ( @PID, 0, 1) -- boost
		insert into Skill values ( @PID, 1, 1) -- behead

		if ( @Class = 1)
			insert into Skill values ( @PID, 4, 1) -- litning

		if( @Specialty = 3)
			insert into Skill values ( @PID, 11, 2) -- rest
		else
			insert into Skill values ( @PID, 11, 1) -- rest

		fetch next from Player_Cursor into @PID, @Class, @Specialty
	end -- while end

	close Player_Cursor
	deallocate Player_Cursor


--	2. 스킬포인트 재분배
	update Player set SUPoint = [Level] - 1

	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 7 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 11 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 12 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 13 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 14 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 16 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 19 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 21 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 22 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 23 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 24 and b.Clear = 1)


--	3. 26번 퀘스트 DB에서 삭제
	delete from Quest where [Quest] = 26
GO
/****** Object:  StoredProcedure [dbo].[USP_040326]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER OFF
GO
CREATE procedure [dbo].[USP_040326]
as
/*
	1. 19 퀘스트에 대한 스킬포인트 재분배
*/


--	1. 19 퀘스트에 대한 스킬포인트 재분배
	declare @PID int
	declare @Class tinyint
	declare @Level tinyint
	declare @SUPoint smallint
	declare @SPSum int
	declare @BonusPoint int
	declare @n int

	set @BonusPoint = 0
	set @n = 0

	declare Player_Cursor cursor for
	select a.PID, a.Class, a.Level, a.SUPoint from Player a, Quest b where a.PID = b.PID and b.Quest = 19 and b.Clear = 1
	open Player_Cursor

	fetch next from Player_Cursor into @PID, @Class, @Level, @SUPoint
	while (@@FETCH_STATUS = 0)
	begin
		if( select count(*) from Quest where PID = @PID and Quest = 7 and Clear = 1) > 0
			set @BonusPoint = @BonusPoint + 1
		if( select count(*) from Quest where PID = @PID and Quest = 11 and Clear = 1) > 0
			set @BonusPoint = @BonusPoint + 1
		if( select count(*) from Quest where PID = @PID and Quest = 12 and Clear = 1) > 0
			set @BonusPoint = @BonusPoint + 1
		if( select count(*) from Quest where PID = @PID and Quest = 13 and Clear = 1) > 0
			set @BonusPoint = @BonusPoint + 1
		if( select count(*) from Quest where PID = @PID and Quest = 14 and Clear = 1) > 0
			set @BonusPoint = @BonusPoint + 1
		if( select count(*) from Quest where PID = @PID and Quest = 16 and Clear = 1) > 0
			set @BonusPoint = @BonusPoint + 1
		if( select count(*) from Quest where PID = @PID and Quest = 19 and Clear = 1) > 0
			set @BonusPoint = @BonusPoint + 1
		if( select count(*) from Quest where PID = @PID and Quest = 21 and Clear = 1) > 0
			set @BonusPoint = @BonusPoint + 1
		if( select count(*) from Quest where PID = @PID and Quest = 22 and Clear = 1) > 0
			set @BonusPoint = @BonusPoint + 1
		if( select count(*) from Quest where PID = @PID and Quest = 23 and Clear = 1) > 0
			set @BonusPoint = @BonusPoint + 1
		if( select count(*) from Quest where PID = @PID and Quest = 24 and Clear = 1) > 0
			set @BonusPoint = @BonusPoint + 1

		select @SPSum = sum( Level) from Skill where PID = @PID and [Index] <> 11

		if @Class = 1
		begin
			if ( @Level - 1 + @BonusPoint) > ( @SUPoint + @SPSum - 3)
			begin
				update Player set SUPoint = SUPoint + 1 where PID = @PID
				set @n = @n + 1
			end
		end
		else	-- else
		begin
			if ( @Level - 1 + @BonusPoint) > ( @SUPoint + @SPSum - 2)
			begin
				update Player set SUPoint = SUPoint + 1 where PID = @PID
				set @n = @n + 1
			end
		end

		set @BonusPoint = 0

		fetch next from Player_Cursor into @PID, @Class, @Level, @SUPoint
	end -- while end

	close Player_Cursor
	deallocate Player_Cursor

	select @n
GO
/****** Object:  StoredProcedure [dbo].[USP_040514]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER OFF
GO
CREATE procedure [dbo].[USP_040514]
as
/*
	1. 스킬초기화
	2. 스킬포인트 재분배
*/


--	1. 스킬초기화
	delete from Skill

	declare @PID int
	declare @Class tinyint
	declare @Level tinyint
	declare @Specialty tinyint

	-- reset skills..
	declare Player_Cursor cursor for
	select [PID], [Class], [Level], [Specialty] from Player
	open Player_Cursor

	fetch next from Player_Cursor into @PID, @Class, @Level, @Specialty
	while (@@FETCH_STATUS = 0)
	begin
		insert into Skill values ( @PID, 0, 1) -- 전력질주
		insert into Skill values ( @PID, 1, 1) -- 참수
		
		-- 직업별 기술 습득
		if ( @Class = 0)
		begin
			if ( @Level >= 18)
				insert into Skill values ( @PID, 4, 1) -- 우롱하기
			if ( @Specialty = 3)
				insert into Skill values ( @PID, 6, 1) -- 막아내기
		end
		else if ( @Class = 1)
		begin
			insert into Skill values ( @PID, 4, 1) -- 낙뢰술
			if ( @Specialty = 3)
				insert into Skill values ( @PID, 17, 1) -- 인염술
		end
		else if ( @Class = 2)
		begin
			if ( @Specialty = 3)
				insert into Skill values ( @PID, 7, 1) -- 뛰어난기회포착
		end

		-- 관직별 기술 습득
		if ( @Specialty = 3)
			insert into Skill values ( @PID, 11, 2) -- 휴식2레벨
		else
			insert into Skill values ( @PID, 11, 1) -- 휴식1레벨

		fetch next from Player_Cursor into @PID, @Class, @Level, @Specialty
	end -- while end

	close Player_Cursor
	deallocate Player_Cursor


--	2. 스킬포인트 재분배
	update Player set SUPoint = [Level] - 1

	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 7 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 11 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 12 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 13 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 14 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 16 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 19 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 21 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 22 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 23 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 24 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 33 and b.Clear = 1)
GO
/****** Object:  StoredProcedure [dbo].[USP_040519]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER OFF
GO
CREATE procedure [dbo].[USP_040519]
as
/*
	1. 노리개(퀘스트아이템) Index값 변경
	2. 스킬 추가
*/


--	1. 노리개(퀘스트아이템) Index값 변경
	declare @PID int
	declare @IID int
	declare @Prefix tinyint
	declare @Info tinyint

	declare i_cur cursor for
	select IID, Prefix from Item where [Index] = 205 and Prefix in ( 80, 81, 82)
	open i_cur
	
	fetch next from i_cur into @IID, @Prefix
	while (@@FETCH_STATUS = 0)
	begin
		if ( @Prefix = 80)
			update Item set [Index] = 202, Prefix = 0 where IID = @IID
		else if ( @Prefix = 81)
			update Item set [Index] = 204, Prefix = 0 where IID = @IID
		else if ( @Prefix = 82)
			update Item set [Index] = 203, Prefix = 0 where IID = @IID
	
		fetch next from i_cur into @IID, @Prefix
	end -- while end
	
	close i_cur
	deallocate i_cur
	
	declare i_cur cursor for
	select PID, IID, Info from Item where [Index] = 205 and Prefix not in ( 80, 81, 82)
	open i_cur
	
	fetch next from i_cur into @PID, @IID, @Info
	while (@@FETCH_STATUS = 0)
	begin
		if ( ( @Info & 16) = 0)
		begin
			declare @Class tinyint
			select @Class = Class from Player where PID = @PID
			if ( @Class = 0)
				update Item set [Index] = 202 where IID = @IID
			else if ( @Class = 1)
				update Item set [Index] = 204 where IID = @IID
			else if ( @Class = 2)
				update Item set [Index] = 203 where IID = @IID
		end
		else
			update Item set [Index] = convert( int, rand() * 3 + 202) where IID = @IID
	
		fetch next from i_cur into @PID, @IID, @Info
	end -- while end
	
	close i_cur
	deallocate i_cur

--	2. 스틸 추가
	insert Skill select PID, [index] = 18, [Level] = 1 from Player where [Level] >= 35  and Class = 1 and Specialty = 3
	insert Skill select PID, [index] = 19, [Level] = 1 from Player where [Level] >= 40  and Class = 1 and Specialty = 3
	insert Skill select PID, [index] = 20, [Level] = 1 from Player where [Level] >= 45  and Class = 1 and Specialty = 3
	insert Skill select PID, [index] = 21, [Level] = 1 from Player where [Level] >= 50  and Class = 1 and Specialty = 3
	
	update Skill set [Level] = [Level] + 1 where Skill.[Index] = 7 and Skill.PID in( select PID from Player where [Level] >= 35 and Class = 2 and Specialty = 3)
	update Skill set [Level] = [Level] + 1 where Skill.[Index] = 7 and Skill.PID in( select PID from Player where [Level] >= 40 and Class = 2 and Specialty = 3)
	update Skill set [Level] = [Level] + 1 where Skill.[Index] = 7 and Skill.PID in( select PID from Player where [Level] >= 45 and Class = 2 and Specialty = 3)
	update Skill set [Level] = [Level] + 1 where Skill.[Index] = 7 and Skill.PID in( select PID from Player where [Level] >= 50 and Class = 2 and Specialty = 3)
GO
/****** Object:  StoredProcedure [dbo].[USP_040616]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER OFF
GO
CREATE procedure [dbo].[USP_040616]
as
/*
	1. 스킬초기화
	2. 스킬포인트 재분배
	3. 공헌도 재분배
	4. 푸른감자 삭제
*/


--	1. 스킬초기화
	delete from Skill

	declare @PID int
	declare @Class tinyint
	declare @Level tinyint
	declare @Specialty tinyint

	-- reset skills..
	declare Player_Cursor cursor for
	select [PID], [Class], [Level], [Specialty] from Player
	open Player_Cursor

	fetch next from Player_Cursor into @PID, @Class, @Level, @Specialty
	while (@@FETCH_STATUS = 0)
	begin
		insert into Skill values ( @PID, 0, 1) -- 전력질주
		insert into Skill values ( @PID, 1, 1) -- 참수
		
		-- 직업별 기술 습득
		if ( @Class = 0)
		begin
			if ( @Level >= 15)
				insert into Skill values ( @PID, 14, 1) -- 보호하기
			if ( @Level >= 18)
				insert into Skill values ( @PID, 4, 1) -- 우롱하기
			if ( @Specialty = 3)
				insert into Skill values ( @PID, 6, 1) -- 막아내기
		end
		else if ( @Class = 1)
		begin
			insert into Skill values ( @PID, 4, 1) -- 낙뢰술
			if ( @Specialty = 3)
			begin
				insert into Skill values ( @PID, 17, 1) -- 인염술
				if( @Level >= 35)
					insert into Skill values ( @PID, 18, 1) -- 인해술
				if( @Level >= 40)
					insert into Skill values ( @PID, 19, 1) -- 인노술
				if( @Level >= 45)
					insert into Skill values ( @PID, 20, 1) -- 인거술
				if( @Level >= 50)
					insert into Skill values ( @PID, 21, 1) -- 인화술
			end
		end
		else if ( @Class = 2)
		begin
			if ( @Specialty = 3 and @Level >= 30)			
				insert into Skill values ( @PID, 7, (@Level - 30) / 5 + 1 ) -- 뛰어난기회포착				
			
		end

		-- 관직별 기술 습득
		if ( @Specialty = 3)
			insert into Skill values ( @PID, 11, 2) -- 휴식2레벨
		else
			insert into Skill values ( @PID, 11, 1) -- 휴식1레벨

		fetch next from Player_Cursor into @PID, @Class, @Level, @Specialty
	end -- while end

	close Player_Cursor
	deallocate Player_Cursor


--	2. 스킬포인트 재분배
	update Player set SUPoint = [Level] - 1
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 7 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 11 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 12 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 13 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 14 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 16 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 19 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 21 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 22 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 23 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 24 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 33 and b.Clear = 1)


--	3. 공헌도 재분배
	update Player set Contribute = 0
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 3 and b.Clear = 1)
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 5 and b.Clear = 1)
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 6 and b.Clear = 1)
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 8 and b.Clear = 1)
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 10 and b.Clear = 1)
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 15 and b.Clear = 1)
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 17 and b.Clear = 1)
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 18 and b.Clear = 1)
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 21 and b.Clear = 1)
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 22 and b.Clear = 1)
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 23 and b.Clear = 1)
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 25 and b.Clear = 1)
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 27 and b.Clear = 1)
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 28 and b.Clear = 1)
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 29 and b.Clear = 1)
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 30 and b.Clear = 1)
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 31 and b.Clear = 1)
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 32 and b.Clear = 1)
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 34 and b.Clear = 1)
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 35 and b.Clear = 1)
	update Player set Contribute = Contribute + 2 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 9005 and b.Clear = 1)
	
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 4 and b.Flag = 4)
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 12 and b.Flag = 3)
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 14 and b.Flag = 7)
	update Player set Contribute = Contribute + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 20 and b.Flag = 2)


--	4. 푸른감자 삭제
	delete from Item where [Index] = 333
GO
/****** Object:  StoredProcedure [dbo].[USP_040630]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER OFF
GO
CREATE procedure [dbo].[USP_040630]
as
/*
	1. 스킬초기화
	2. 스킬포인트 재분배
	3. 푸른감자 삭제
*/


--	1. 스킬초기화
	delete from Skill

	declare @PID int
	declare @Class tinyint
	declare @Level tinyint
	declare @Specialty tinyint

	-- reset skills..
	declare Player_Cursor cursor for
	select [PID], [Class], [Level], [Specialty] from Player
	open Player_Cursor

	fetch next from Player_Cursor into @PID, @Class, @Level, @Specialty
	while (@@FETCH_STATUS = 0)
	begin
		insert into Skill values ( @PID, 0, 1) -- 전력질주
		insert into Skill values ( @PID, 1, 1) -- 참수
		
		-- 직업별 기술 습득
		if ( @Class = 0)
		begin
			if ( @Level >= 15)
				insert into Skill values ( @PID, 14, 1) -- 보호하기
			if ( @Level >= 18)
				insert into Skill values ( @PID, 4, 1) -- 우롱하기
			if ( @Specialty = 3)
				insert into Skill values ( @PID, 6, 1) -- 막아내기
		end
		else if ( @Class = 1)
		begin
			insert into Skill values ( @PID, 4, 1) -- 낙뢰술
			if ( @Specialty = 3)
			begin
				insert into Skill values ( @PID, 17, 1) -- 인염술
				if( @Level >= 35)
					insert into Skill values ( @PID, 18, 1) -- 인해술
				if( @Level >= 40)
					insert into Skill values ( @PID, 19, 1) -- 인노술
				if( @Level >= 45)
					insert into Skill values ( @PID, 20, 1) -- 인거술
				if( @Level >= 50)
					insert into Skill values ( @PID, 21, 1) -- 인화술
			end
		end
		else if ( @Class = 2)
		begin
			if ( @Specialty = 3 and @Level >= 30)			
				insert into Skill values ( @PID, 7, (@Level - 30) / 5 + 1 ) -- 뛰어난기회포착				
			
		end

		-- 관직별 기술 습득
		if ( @Specialty = 3)
			insert into Skill values ( @PID, 11, 2) -- 휴식2레벨
		else
			insert into Skill values ( @PID, 11, 1) -- 휴식1레벨

		fetch next from Player_Cursor into @PID, @Class, @Level, @Specialty
	end -- while end

	close Player_Cursor
	deallocate Player_Cursor


--	2. 스킬포인트 재분배
	update Player set SUPoint = [Level] - 1
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 7 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 11 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 12 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 13 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 14 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 16 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 19 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 21 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 22 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 23 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 24 and b.Clear = 1)
	update Player set SUPoint = a.SUPoint + 1 from Player a, Quest b where a.PID = b.PID and ( b.Quest = 33 and b.Clear = 1)


--	3. 푸른감자 삭제
	delete from Item where [Index] = 333
GO
/****** Object:  StoredProcedure [dbo].[USP_040908]    Script Date: 02.10.2018 13:53:21 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER OFF
GO
CREATE procedure [dbo].[USP_040908]
as
/*
	1. 이벤트보상
*/


--	1. 이벤트보상
	declare @MaxIID int
	select @MaxIID = max( IID) from Item

	declare @PID int
	declare @Level tinyint
	declare @IID int
	declare @Money int

	declare Player_Cursor cursor for
	select [PID], [Level] from Player where [UID] >= 530934 and [Level] >= 10
	open Player_Cursor

	fetch next from Player_Cursor into @PID, @Level
	while (@@FETCH_STATUS = 0)
	begin
		if( @Level >= 20)
			set @Money = 30000
		else if( @Level >= 15)
			set @Money = 20000
		else
			set @Money = 10000

		set @IID = 0
		select @IID = [IID] from Item where [PID] = @PID and [Index] = 31 and ( [Info] & 16) = 0
		if( @IID = 0)
		begin
			set @MaxIID = @MaxIID + 1
			insert into Item values ( @PID, @MaxIID, 31, 0, 0, @Money, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
		end
		else
			update Item set [Num] = [Num] + @Money where [IID] = @IID

		fetch next from Player_Cursor into @PID, @Level
	end -- while end

	close Player_Cursor
	deallocate Player_Cursor
GO
USE [master]
GO
ALTER DATABASE [kal_db] SET  READ_WRITE 
GO
