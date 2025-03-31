SELECT 
    t.lessonNum, 
    t.beginTime, 
    t.endTime, 
    g.lesson, 
    g.type 
FROM 
    (SELECT time.dayType, time.lessonNum, time.beginTime, time.endTime
     FROM [time]
     WHERE (((time.dayType) = "Будні дні"))
    ) AS t
LEFT JOIN 
    (SELECT [КІ-21д].num, [КІ-21д].lesson, [КІ-21д].numWeek, [КІ-21д].type
     FROM [КІ-21д]
     WHERE [КІ-21д].dayWeek = "Понеділок" AND [КІ-21д].numWeek LIKE '%1,%'
    ) AS g
ON 
    t.lessonNum = g.num
WHERE 
    g.lesson IS NOT NULL
ORDER BY 
    t.lessonNum;