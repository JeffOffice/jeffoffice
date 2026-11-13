# Verificar ortografia

<!-- keywords: ortografia, verificação ortográfica, corretor ortográfico, palavra errada, sublinhado vermelho, linha ondulada, sugestões, ignorar, ignorar tudo, adicionar ao dicionário, dicionário personalizado, dicionário, idioma, revisão de texto, F7, revisão, verificar ortografia ao digitar, maiúsculas, números, gramática, verificar gramática, estilo, sublinhado azul, linha pontilhada, verificar gramática ao digitar -->

O JeffWrite verifica a sua ortografia enquanto você digita. Uma palavra que não está no dicionário recebe um sublinhado vermelho ondulado. A verificação é feita no seu próprio computador, sem conexão com a internet, e conhece 31 idiomas e variantes regionais.

## Enquanto você digita

- Um sublinhado vermelho ondulado significa que o dicionário não tem a palavra. A palavra pode estar correta mesmo assim. Nomes e palavras novas costumam não constar no dicionário. Os nomes dos aplicativos do JeffOffice (JeffWrite, JeffPub, JeffDeck, JeffOffice, JeffCrunch, JeffMail e JeffBase) e LLC nunca são marcados, em nenhum idioma, com ou sem "s".
- A palavra que você está digitando não é marcada até você passar para a próxima, então uma palavra ainda incompleta não fica piscando.
- Os sublinhados aparecem somente na tela. Eles nunca são impressos e nunca vão para um PDF.

Para corrigir uma palavra, clique nela com o **botão direito**. O menu mostra estas opções acima das habituais:

- Até seis **sugestões**, a melhor primeiro, em negrito. Clique em uma para substituir a palavra. É uma única etapa, então **Ctrl+Z** a desfaz.
- **Ignorar Tudo** deixa essa palavra de lado, onde quer que apareça, até você fechar o JeffWrite.
- **Adicionar ao Dicionário** ensina a palavra ao JeffWrite de forma permanente. Ela é gravada em um pequeno arquivo na pasta do próprio JeffWrite, então você pode ler ou editar sua lista de palavras adicionadas ali.

Se o dicionário não tiver nenhuma sugestão, o menu informa isso.

## Percorrer os erros de ortografia

Pressione **F7** ou clique em **Revisão > Ortografia**. O painel Ortografia abre à direita. Ele mostra a primeira palavra com erro após o cursor, selecionada no seu texto, com a linha em que ela está e uma lista de sugestões.

- **Alterar** substitui a palavra pela sugestão escolhida. Clique duas vezes em uma sugestão para fazer o mesmo.
- **Alterar Tudo** faz isso para cada ocorrência da palavra no documento. É uma única etapa, então **Ctrl+Z** desfaz todas.
- **Ignorar** deixa esta palavra de lado. O sublinhado vermelho permanece.
- **Ignorar Tudo** e **Adicionar ao Dicionário** funcionam como no menu do botão direito.

O painel vai até o fim do documento, depois continua desde o início, até o ponto onde começou. Quando não restarem mais erros, ele informa que a verificação ortográfica foi concluída. Quando a ortografia termina, o painel passa para os problemas de gramática e estilo (veja Verificar gramática, abaixo).

## Ativar ou desativar os sublinhados vermelhos

Clique em **Revisão > Verificar Ortografia ao Digitar**. A mesma opção está em **Arquivo > Opções > Revisão de Texto**, e é a única opção compartilhada por todos os aplicativos do JeffOffice. Ali você também pode escolher ignorar palavras em MAIÚSCULAS e palavras com números, como 3ª. Ambas são ignoradas por padrão.

## Verificar gramática ao digitar

O JeffWrite também procura erros claros de gramática e de estilo em textos em inglês. Um problema recebe uma linha azul pontilhada. A ortografia mantém o sublinhado vermelho ondulado, e uma palavra com erro de ortografia nunca é marcada como problema de gramática. Estes são os erros que ele procura:

- uma palavra repetida ("the the")
- "a" e "an" antes do som errado ("an test", "a hour")
- uma frase que começa com letra minúscula depois de um ponto
- "its" e "it's", "their" e "there", "your" e "you're", "then" e "than", e "lose" e "loose", quando as palavras ao redor deixam o erro claro
- "could of" em vez de "could have"
- dois espaços seguidos dentro de uma frase
- um espaço antes de vírgula, ponto ou outro sinal que pertence à palavra anterior
- nenhum espaço depois do fim de uma frase
- "alot", e "everyday" em vez de "every day" ("almost everyday")

O programa prefere deixar passar um erro a marcar um texto correto, então uma frase sem sublinhado azul não é garantia de que esteja correta.

Clique com o botão direito sobre o problema. O menu mostra o que está errado e, em seguida, uma ou mais sugestões. Clique em uma sugestão para colocá-la no texto, e pressione Ctrl+Z para desfazer. **Ignorar Este Problema** remove o sublinhado apenas neste ponto, até você fechar o JeffWrite. As mesmas palavras em outro lugar mantêm o sublinhado, e editar outras partes do parágrafo não o faz voltar. **Ignorar Tudo** remove o sublinhado de todos os problemas com esta mensagem nestas palavras.

A verificação é feita no seu próprio computador, sem conexão com a internet, em segundo plano: a digitação nunca é interrompida, e um parágrafo é verificado logo depois que você para de digitar nele. Um parágrafo é verificado até os seus primeiros 100.000 caracteres. Um documento que exige mais verificação do que o limite deixa de ser verificado, e a barra de status informa isso uma vez; a verificação ortográfica continua funcionando.

Somente o inglês é verificado. Em um parágrafo com mais de um idioma, o texto em inglês é verificado e o restante é ignorado, assim como o texto que um arquivo marca como "não verificar". A gramática é verificada no texto do documento, mas ainda não em tabelas, cabeçalhos, rodapés e notas.

Para desativar, clique em **Revisão > Verificar Gramática ao Digitar**. A mesma opção está em **Arquivo > Opções > Revisão de Texto**. Somente o JeffWrite tem essa opção, e ela é a mesma em todas as janelas do JeffWrite. O painel Ortografia (F7) percorre os problemas de gramática depois da ortografia, quando esta opção está ativada.

## Idiomas

O JeffWrite verifica cada trecho de texto no seu próprio idioma. Um arquivo pode indicar em que idioma está o texto, até uma única palavra, e o JeffWrite lê essa indicação. Um texto sem indicação é verificado no idioma do próprio documento. O arquivo define esse idioma para o documento inteiro, e um documento novo usa o inglês dos Estados Unidos.

Um texto em um idioma para o qual o JeffWrite não tem dicionário não é verificado. Também não é verificado o texto que um arquivo marca como "não verificar ortografia". Você ainda não pode mudar o idioma de um texto no JeffWrite, mas ele mantém o que o arquivo indica.

Os 31 dicionários são: inglês (americano, britânico, canadense e australiano), espanhol (21 regiões, incluindo México e Espanha), francês, alemão, italiano, holandês e português (Brasil e Portugal).

## O que ainda não é verificado

Os sublinhados vermelhos aparecem no texto e nas células de tabelas. Eles ainda não aparecem em cabeçalhos, rodapés, notas ou caixas de texto, e o painel Ortografia também não verifica esses elementos.

## Veja também

- [Bem-vindo ao JeffWrite](start)
- [Imprimir](print)
