# Notas, comentários e links
<!-- keywords: painel de comentários, novo comentário, responder, resolver, reabrir, discussão, comentário anterior, próximo comentário, mostrar comentários, converter anotações em comentários, converter comentários em anotações, ortografia, verificar ortografia, palavra incorreta, F7, ignorar tudo, alterar tudo, adicionar ao dicionário, dicionário de sinônimos, sinônimos, lista de palavras, nota, notas, anotação, anotações, comentário, comentários, passar o ponteiro, marca vermelha, canto, nova anotação, editar anotação, excluir anotação, mostrar anotação, ocultar anotação, mostrar todas as anotações, link, links, hiperlink, endereço, página da web, email, e-mail, local nesta pasta de trabalho, Ctrl+K, Ctrl+clique, dica de tela, abrir link, remover link, conversa, estatísticas da pasta de trabalho, contar, células, fórmulas, gráficos, tabelas dinâmicas, acessibilidade, texto alternativo, células mescladas, arquivos recentes -->

## Anotações

Uma **anotação** é um texto curto associado a uma célula. Uma célula que tem uma anotação mostra uma pequena marca vermelha no canto superior direito. Ao apontar para a célula, a anotação aparece ao lado dela; ao afastar o ponteiro, ela desaparece.

Para colocar uma anotação em uma célula, selecione-a e use **Revisão > Nova Anotação** (ou pressione **Shift+F2**). Digite o texto e clique em **OK**. Em uma célula que já tem anotação, o mesmo comando aparece como **Editar Anotação**. Apagar o texto remove a anotação.

- **Excluir Anotação** remove as anotações das células selecionadas.
- **Mostrar ou Ocultar Anotação** mantém a anotação das células selecionadas sempre visível na planilha, em uma caixa ao lado de cada célula, ou a oculta novamente.
- **Mostrar Todas as Anotações** faz o mesmo para todas as anotações da planilha; o mesmo botão passa a se chamar **Ocultar Todas as Anotações**.
- **Anotação Anterior** e **Próxima Anotação** levam de uma célula com anotação para a seguinte.

Cada uma dessas ações pode ser desfeita em um único passo. A anotação acompanha a sua célula quando linhas ou colunas são inseridas ou excluídas e quando as células são classificadas. **Limpar Tudo** remove as anotações e os comentários das células que ela limpa.

Uma célula também pode conter uma conversa de **comentários**, com respostas. Veja a próxima parte.

As anotações podem ser salvas em uma pasta de trabalho .xlsx. Uma pasta de trabalho aberta a partir de um tipo de arquivo mais antigo ou de um arquivo de texto mantém as anotações somente depois que você a salvar como pasta de trabalho .xlsx.

## Comentários

Um **comentário** é parecido com uma anotação, mas inicia uma **discussão**: as pessoas podem responder a ele, e uma delas pode marcar a discussão como **resolvida** quando a pergunta for respondida. Cada comentário mostra quem o escreveu e quando. Uma célula com discussão tem a mesma marca vermelha de uma célula com anotação. Ao apontar para a célula, você lê a discussão inteira, com cada resposta recuada abaixo do comentário a que responde. Uma discussão resolvida começa com a palavra **Resolvido**.

Na guia **Revisão**, no grupo **Comentários**:

- **Novo Comentário** (**Ctrl+Shift+F2**) inicia uma discussão na célula selecionada. Digite o texto e clique em **OK**. Funciona em uma célula que não tem anotação nem comentários.
- **Responder** adiciona o seu texto à discussão da célula selecionada.
- **Resolver Tópico** marca a discussão como concluída. O mesmo botão passa a se chamar **Reabrir Discussão**, que a abre novamente.
- **Excluir Comentário** remove as discussões das células selecionadas.
- **Comentário Anterior** e **Próximo Comentário** levam de uma célula com discussão para a seguinte.
- **Mostrar Comentários** abre o painel **Comentários** ao lado da planilha. Ele lista cada discussão da planilha como um cartão: a célula, cada comentário com o autor e a hora, e os botões **Responder**, **Resolver** ou **Reabrir**, e **Excluir**. **Ir Para** leva até a célula. O painel acompanha todas as alterações, inclusive o **Desfazer**.
- **Converter Anotações em Comentários** transforma cada anotação simples da planilha em uma discussão com um comentário. **Converter Comentários em Anotações** transforma cada discussão de volta em uma anotação. Uma anotação não tem respostas, por isso os comentários de uma discussão se tornam uma única anotação com todo o texto, e o JeffCrunch pede confirmação antes de fazer isso com uma discussão que tenha respostas.

Cada uma dessas ações pode ser desfeita em um único passo.

Programas que não conseguem mostrar discussões exibem uma anotação simples no lugar delas. O JeffCrunch mantém essa anotação, com o texto da discussão, a cada vez que salva. Discussões, respostas e as pessoas que as escreveram permanecem como estavam quando você salva e abre a pasta de trabalho novamente. Uma discussão com mais de 500 comentários é exibida e mantida, mas o JeffCrunch não acrescenta nada a ela nem a altera.

Os comentários são salvos em uma pasta de trabalho .xlsx. Uma pasta de trabalho aberta a partir de um tipo de arquivo mais antigo ou de um arquivo de texto mantém os comentários somente depois que você a salvar como pasta de trabalho .xlsx.

## Links

Um **link** leva você a outro lugar quando você o segue: a uma página da web, a um arquivo, a um endereço de email ou a um local nesta pasta de trabalho (uma célula, um intervalo de células ou um nome).

Para colocar um link em uma célula, selecione-a e use **Inserir > Link** (ou pressione **Ctrl+K**). Escolha para onde ele leva, digite o endereço ou o local (ou escolha um dos arquivos abertos recentemente em **Ou um arquivo recente**) e, se quiser, o texto que a célula deve mostrar e uma **dica de tela**. Em uma célula que já tem link, o mesmo comando aparece como **Editar Link**. **Remover Link** retira os links das células selecionadas.

Aponte para uma célula com link para ver para onde ele leva. Para segui-lo, mantenha **Ctrl** pressionada e clique na célula, ou selecione a célula e use **Abrir Link** no menu de clique com o botão direito. Um clique simples apenas seleciona a célula.

O JeffCrunch nunca abre nada por conta própria. Uma página da web ou um endereço de email é aberto no seu programa habitual quando você segue o link. Um link para um arquivo, ou para outro tipo de endereço, pede a sua confirmação antes. Um link que executaria um script nunca é aberto.

Um link acompanha a sua célula quando linhas ou colunas são inseridas ou excluídas e quando as células são classificadas, e é salvo com a pasta de trabalho.

## Estatísticas da Pasta de Trabalho

**Estatísticas da Pasta de Trabalho** (guia **Revisão**) conta o que a pasta de trabalho contém: quantas **Planilhas** ela tem, quantas **Células com dados** (células que contêm um número, um texto ou uma fórmula), quantas **Fórmulas**, quantos **Gráficos** e quantas **Tabelas dinâmicas**. As planilhas excluídas não são contadas. Os mesmos números aparecem na página **Informações** da guia Arquivo.

## Ortografia

**Ortografia** (guia **Revisão**, ou **F7**) verifica as palavras das suas células de texto, uma palavra incorreta por vez. Se você tiver selecionado mais de uma célula, ele verifica somente essas células. Caso contrário, verifica a planilha inteira. Ele não analisa fórmulas, números nem datas.

A janela mostra a palavra que não está no dicionário, uma caixa chamada **Alterar para** com a melhor sugestão e uma lista de outras sugestões. Escolha uma sugestão ou digite a sua própria palavra e, em seguida, escolha:

- **Alterar** coloca a nova palavra no lugar da incorreta naquela célula. Cada alteração pode ser desfeita em um único passo.
- **Alterar Tudo** coloca a nova palavra no lugar da mesma palavra em todas as células verificadas, como um único passo que pode ser desfeito.
- **Ignorar** mantém a palavra como está desta vez. **Ignorar Tudo** a mantém como está em todos os lugares até você fechar o JeffCrunch.
- **Adicionar** coloca a palavra no seu dicionário pessoal, para que ela nunca mais seja marcada. O seu dicionário é compartilhado com os outros aplicativos do JeffOffice.

**Arquivo > Opções > Revisão de Texto** define se palavras em letras maiúsculas e palavras com números são verificadas. O JeffCrunch verifica no idioma em que é exibido, se houver um dicionário para ele, e em inglês americano caso contrário.

Enquanto você digita em uma célula, uma palavra que não está no dicionário recebe uma linha vermelha ondulada embaixo. Clique com o botão direito na palavra para escolher uma correção ou para escolher **Ignorar Tudo** ou **Adicionar ao Dicionário**. As células da grade não são sublinhadas, e uma entrada que começa com sinal de igual nunca é verificada.

## Dicionário de Sinônimos

**Dicionário de Sinônimos** (guia **Revisão**, ou **Shift+F7**) encontra palavras de sentido quase igual ao de uma palavra de uma célula. Ele procura a primeira palavra do texto da célula em que você está. Digite outra palavra em **Pesquisar** e pressione **Enter** para pesquisar essa palavra. Escolha uma palavra na lista e clique em **Substituir**, ou clique duas vezes nela. A palavra da célula é substituída, e uma letra inicial maiúscula continua maiúscula. **Pesquisar** procura a palavra escolhida, para seguir uma cadeia de significados. A lista de palavras funciona em inglês.

## Verificar Acessibilidade

**Verificar Acessibilidade** (guia **Revisão**) abre uma pequena janela que lista o que na pasta de trabalho é difícil de usar para alguns leitores, ou difícil de ser lido em voz alta por um leitor de tela (um programa que lê a tela em voz alta). Cada linha começa com **Erro**, **Aviso** ou **Dica**. Clique em uma linha para ir até a célula.

Ele procura:

- Um gráfico ou uma imagem sem **texto alternativo** (as palavras que descrevem o que ele mostra). Isso é um erro.
- Uma tabela sem linha de cabeçalho, de modo que suas colunas não têm nomes. Isso é um erro.
- **Células mescladas**, que são difíceis de percorrer com o teclado. Isso é um aviso.
- Texto com pouco contraste em relação à cor de fundo, como texto cinza-claro sobre branco. O JeffCrunch mede o quanto o texto se destaca do preenchimento. Textos pequenos precisam de uma proporção de pelo menos 4,5 para 1, e textos grandes (18 pontos, ou negrito de 14 pontos) precisam de 3 para 1. Ele indica a primeira célula de cada combinação de fonte e preenchimento. Isso é um aviso.
- Uma planilha que ainda tem o nome que recebeu, como Planilha1. Isso é uma dica.

Pressione **Verificar Novamente** depois de fazer alterações.
