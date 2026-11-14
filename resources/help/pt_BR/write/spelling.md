# Verificar a ortografia

<!-- keywords: ortografia, verificar ortografia, corretor ortográfico, palavra errada, erro de ortografia, linha vermelha, linha ondulada, sugestões, ignorar, ignorar tudo, adicionar ao dicionário, dicionário personalizado, dicionário, idioma, revisão de texto, f7, revisão, verificar ortografia ao digitar, maiúsculas, números, gramática, verificar gramática, estilo, linha azul, linha pontilhada, verificar gramática ao digitar -->

O JeffWrite verifica sua ortografia enquanto você digita. Uma palavra que não está no dicionário recebe uma linha vermelha ondulada embaixo dela. A verificação é feita no seu próprio computador, sem conexão com a internet, e reconhece 31 idiomas e variantes regionais.

## Enquanto você digita

- Uma linha vermelha ondulada significa que a palavra não está no dicionário. Mesmo assim, a palavra pode estar certa. Nomes e palavras novas costumam estar ausentes. Os nomes dos aplicativos do JeffOffice (JeffWrite, JeffPub, JeffDeck, JeffOffice, JeffCrunch, JeffMail e JeffBase) e "LLC" nunca são marcados, em nenhum idioma, com ou sem "s".
- A palavra que você está digitando não é verificada até você passar para outra, para que uma palavra incompleta não fique piscando.
- As linhas aparecem apenas na tela. Elas nunca são impressas e nunca vão para um PDF.

Para corrigir uma palavra, clique com o **botão direito** nela. O menu mostra estas opções acima das usuais:

- Até seis **sugestões**, a melhor primeiro, em negrito. Clique em uma para substituir a palavra. É uma única etapa, então **Ctrl+Z** a desfaz.
- **Ignorar Tudo** deixa essa palavra de lado, em qualquer lugar onde ela apareça, até você fechar o JeffWrite.
- **Adicionar ao Dicionário** ensina a palavra ao JeffWrite de forma permanente. A palavra é gravada em um pequeno arquivo na pasta do JeffWrite, então você pode ler ou editar sua lista de palavras adicionadas ali.

Se o dicionário não tiver nenhuma sugestão, o menu informa isso.

## Percorrer os erros de ortografia

Pressione **F7** ou clique em **Revisão > Ortografia**. O painel Ortografia abre à direita. Ele mostra a primeira palavra errada depois do cursor, selecionada no seu texto, com a linha em que ela está e uma lista de sugestões.

- **Alterar** substitui a palavra pela sugestão escolhida. Clique duas vezes em uma sugestão para fazer o mesmo.
- **Alterar Tudo** faz isso para todas as ocorrências da palavra no documento. É uma única etapa, então **Ctrl+Z** desfaz todas elas.
- **Ignorar** passa por cima desta palavra. A linha vermelha dela continua.
- **Ignorar Tudo** e **Adicionar ao Dicionário** funcionam como no menu do botão direito.

O painel vai até o fim do documento, depois continua desde o início, até o ponto onde começou. Quando não restar mais nenhum erro, ele informa que a verificação ortográfica foi concluída. Quando a ortografia termina, o painel passa para os problemas de gramática e estilo, se a verificação de gramática estiver ativada (veja Verificar gramática, abaixo).

## Ligar ou desligar as linhas vermelhas

Clique em **Revisão > Verificar Ortografia ao Digitar**. A mesma opção está em **Arquivo > Opções > Revisão de Texto**, e é a única opção usada por todos os aplicativos do JeffOffice. Ali você também pode escolher ignorar palavras em MAIÚSCULAS e palavras com números, como 3º. As duas são ignoradas no início.

## Verificar gramática ao digitar

O JeffWrite também procura erros claros de gramática e estilo no texto. Um problema recebe uma linha azul pontilhada embaixo. A ortografia mantém sua linha vermelha ondulada, e uma palavra escrita errado nunca é marcada como problema de gramática. Estes são os erros que ele procura:

- uma palavra repetida ("the the")
- "a" e "an" antes do som errado ("an test", "a hour")
- uma frase que começa com letra minúscula depois de um ponto
- "its" e "it's", "their" e "there", "your" e "you're", "then" e "than", e "lose" e "loose", quando as palavras ao redor deixam o erro claro
- "could of" em vez de "could have"
- dois espaços seguidos dentro de uma frase
- um espaço antes de vírgula, ponto ou outro sinal que pertence à palavra anterior
- falta de espaço depois do fim de uma frase
- "alot", e "everyday" em vez de "every day" ("almost everyday")
- verbos que não concordam com o sujeito em casos claros ("she don't", "they was", "it have", "everyone are", "there is many reasons")
- a forma errada de um verbo ("did went", "has went", "can't hardly", "don't have no")
- palavras trocadas ("who's car", "an affect on", "less people", "irregardless", "can't breath", "weather" em vez de "whether", "chose" em vez de "choose", "advise" em vez de "advice")
- "between you and I", "Me and him went", "more better" e "these thing"
- uma vírgula depois do ano de uma data ("On June 5, 2020, we left"), uma vírgula depois de cidade e estado ("in Austin, Texas, the heat"), e um hífen em um composto antes de um substantivo ("a well-known author")
- um "i" minúsculo no lugar de "I", e os dias da semana e os meses em letra minúscula

Ele prefere deixar passar um erro a marcar um texto que está certo, então uma frase sem linha azul não é garantia de que esteja correta.

Clique com o botão direito no problema. O menu mostra o que está errado e, em seguida, uma ou mais sugestões. Clique em uma sugestão para colocá-la no texto e pressione Ctrl+Z para desfazê-la. **Ignorar Este Problema** remove a linha apenas neste lugar, até você fechar o JeffWrite. As mesmas palavras em outro lugar mantêm a linha, e editar outra parte do parágrafo não a faz voltar. **Ignorar Tudo** remove a linha de todos os problemas com esta mensagem nestas palavras.

A verificação é feita no seu próprio computador, sem conexão com a internet, em segundo plano: a digitação nunca é interrompida, e um parágrafo é verificado logo depois que você para de digitar nele. Um parágrafo é verificado até os primeiros 100.000 caracteres. Um documento que exige mais verificação do que o limite deixa de ser verificado, e a barra de status informa isso uma vez; a ortografia continua funcionando.

**Quais idiomas são verificados.** Inglês, francês, alemão e espanhol, cada um com regras próprias. O francês verifica o espaço sem quebra antes de ; : ! ? e dentro de « », o apóstrofo em palavras como "l'ami" e "qu'il", e trocas como "a" e "à". O alemão verifica "das" e "dass", "seid" e "seit", "wider" e "wieder", a vírgula antes de "dass" e "weil", e letras maiúsculas em substantivos comuns. O espanhol procura um ¿ ou ¡ que falte, acentos em palavras interrogativas, "a ver" e "haber", e "del" e "al". Um parágrafo é verificado no idioma em que está marcado, e texto sem idioma é verificado como inglês. Em um parágrafo com mais de um idioma, usa-se o primeiro idioma verificado e o restante é ignorado. Texto que um arquivo marca como "não verificar" também é ignorado. A gramática é verificada no texto do documento, ainda não em tabelas, cabeçalhos, rodapés e notas.

Para desligar, clique em **Revisão > Verificar Gramática ao Digitar**. A mesma opção está em **Arquivo > Opções > Revisão de Texto**. É uma única opção para todos os aplicativos do JeffOffice, e é a mesma em todas as janelas. Quando ela está ativada, o painel Ortografia (F7) percorre os problemas de gramática depois da ortografia.

**Escolha o que ele procura.** Clique em **Arquivo > Opções > Revisão de Texto > Configurações de gramática** para escolher os tipos de problema: **Gramática** (verbos que não concordam, forma errada de um verbo, "a" e "an"), **Pontuação** (espaços, vírgulas e hífens extras ou ausentes), **Maiúsculas e minúsculas**, **Palavras trocadas** (palavras que soam parecido), **Repetição** e **Estilo**. **Estilo** fica desativado até você ativá-lo. Ele faz sugestões, e não aponta erros: frases de mais de 40 palavras, a voz passiva, três frases seguidas que começam com a mesma palavra, e expressões prolixas ("in order to", "due to the fact that"). A escolha é mantida quando você clica em **OK** nas Opções. Ela se aplica de imediato às linhas azuis e ao painel Ortografia (F7) nas janelas abertas deste aplicativo, e para os outros aplicativos do JeffOffice na próxima vez que forem iniciados.

## Idiomas

O JeffWrite verifica cada trecho de texto no seu próprio idioma. Um arquivo pode informar em qual idioma está o texto, até uma única palavra, e o JeffWrite lê essa informação. Texto sem informação é verificado no idioma do documento. O arquivo define esse idioma para o documento inteiro, e um documento novo usa inglês americano.

Texto em um idioma para o qual o JeffWrite não tem dicionário não é verificado. Também não é verificado o texto que um arquivo marca como "não verificar ortografia". Você ainda não pode mudar o idioma de um texto no JeffWrite, mas ele mantém o que o arquivo indica.

Os 31 dicionários são: inglês (americano, britânico, canadense e australiano), espanhol (21 regiões, incluindo México e Espanha), francês, alemão, italiano, holandês e português (Brasil e Portugal).

## O que ainda não é verificado

As linhas vermelhas aparecem no texto e nas células das tabelas. Elas ainda não aparecem em cabeçalhos, rodapés, notas ou caixas de texto, e o painel Ortografia também não verifica esses elementos.

## Relacionados

- [Bem-vindo ao JeffWrite](start)
- [Imprimir](print)
