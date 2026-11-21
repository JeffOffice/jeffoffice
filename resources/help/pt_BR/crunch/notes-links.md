# Notas, comentários e links
<!-- keywords: remover hiperlinks, remover aparência de link, mover nota, dimensionar nota, redimensionar nota, caixa de nota, redefinir tamanho da nota, teclado, idioma, idioma de revisão de texto, definir idioma, não verificar ortografia, traduzir, tradução, libretranslate, painel de comentários, novo comentário, responder, resolver, reabrir, discussão, comentário anterior, próximo comentário, mostrar comentários, converter anotações em comentários, converter comentários em anotações, ortografia, verificação ortográfica, palavra incorreta, F7, ignorar tudo, alterar tudo, adicionar ao dicionário, dicionário de sinônimos, sinônimos, lista de palavras, nota, notas, comentário, comentários, passar o ponteiro, marca vermelha, canto, nova nota, editar nota, excluir nota, mostrar nota, ocultar nota, mostrar todas as notas, link, links, hiperlink, endereço, página da web, email, lugar nesta pasta de trabalho, Ctrl+K, Ctrl+clique, dica de tela, abrir link, remover link, conversa, estatísticas da pasta de trabalho, contar, células, fórmulas, gráficos, tabelas dinâmicas, acessibilidade, texto alternativo, células mescladas, arquivos recentes -->

## Notas

Uma **nota** é um pequeno texto associado a uma célula. Uma célula que tem nota mostra uma pequena marca vermelha no canto superior direito. Ao apontar para a célula, a nota aparece ao lado dela; ao tirar o ponteiro, ela some.

Para colocar uma nota em uma célula, selecione-a e use **Revisão > Nova Nota** (ou pressione **Shift+F2**). Digite o texto e clique em **OK**. Em uma célula que já tem nota, o mesmo comando aparece como **Editar Nota**. Apagar o texto remove a nota.

- **Excluir Nota** remove as notas das células selecionadas.
- **Mostrar ou Ocultar Nota** mantém a nota das células selecionadas visível na planilha o tempo todo, em uma caixa ao lado de cada célula, ou volta a ocultá-la.
- **Mostrar Todas as Notas** faz o mesmo para todas as notas da planilha; o mesmo botão então aparece como **Ocultar Todas as Notas**.
- **Nota Anterior** e **Próxima Nota** passam de uma célula com nota para a seguinte.
- **Mover ou Redimensionar Nota** move ou redimensiona com as teclas a caixa de uma nota visível. Selecione a célula e use o comando (**Revisão > Anotações**, ou clique com o botão direito na célula). Depois, as **teclas de seta** movem a caixa em 10 pixels; **Shift com uma seta** a torna mais larga, mais estreita, mais alta ou mais baixa em 10 pixels; e **Ctrl** (**Option** em um Mac) faz os passos de 1 pixel. Combinações com Alt ou Meta (ou Command em um Mac) não fazem nada nesse momento, e qualquer outro comando, a troca de planilha, um Desfazer ou um Refazer faz a caixa ser abandonada. **Enter** mantém a caixa onde está, como uma única etapa que você pode desfazer, e **Esc** desiste da alteração. Um leitor de tela informa a posição da caixa após cada tecla, em pixels com zoom de 100% (quanto está à direita e abaixo do canto superior esquerdo da célula, e a largura e a altura da caixa), e informa quando você solta uma tecla mantida pressionada, não a cada repetição; também informa quando a caixa é mantida ou abandonada. Uma caixa tem no máximo 640 pixels de largura e 480 de altura, e começa a até 1.000 pixels de sua célula, como ocorre ao arrastá-la. Arrastar a caixa e suas oito alças com o ponteiro produz o mesmo resultado.
- **Redefinir Tamanho da Nota** devolve a caixa da nota ao padrão: ao lado da célula, com o tamanho necessário para o seu texto.

Cada um desses itens é uma etapa que você pode desfazer. Uma nota acompanha a sua célula quando linhas ou colunas são inseridas ou excluídas e quando as células são classificadas. **Limpar Tudo** remove as notas e os comentários das células que limpa.

Uma célula também pode ter uma conversa de **comentários**, com respostas. Veja a próxima parte.

As notas podem ser salvas em uma pasta de trabalho .xlsx, em uma pasta de trabalho .ods e em uma pasta de trabalho .xls. Uma pasta de trabalho aberta a partir de um arquivo .ods ou .xls tem as notas exibidas, e você pode adicioná-las, alterá-las, movê-las e excluí-las como em uma pasta de trabalho .xlsx. A data de uma nota .ods não é mantida quando o JeffCrunch salva. Uma conversa de comentários salva como arquivo .ods ou .xls se torna uma nota simples com o seu texto, e continua sendo uma nota simples ao abrir esse arquivo. Um arquivo .xls guarda a distância de uma caixa até a célula em um tamanho fixo; por isso, uma caixa que fica muito afastada dentro de uma célula larga pode se deslocar um pouco. Uma pasta de trabalho aberta a partir de um arquivo de texto não recebe notas.

## Comentários

Um **comentário** é parecido com uma nota, mas inicia uma **discussão**: as pessoas podem responder a ele, e uma delas pode marcar a discussão como **resolvida** quando a pergunta for respondida. Cada comentário mostra quem o escreveu e quando. Uma célula com discussão tem a mesma marca vermelha de uma célula com nota. Aponte para a célula para ler a discussão inteira, com cada resposta recuada abaixo do comentário que ela responde. Uma discussão resolvida começa com a palavra **Resolvido**.

Na guia **Revisão**, no grupo **Comentários**:

- **Novo Comentário** (**Ctrl+Shift+F2**) inicia uma discussão na célula selecionada. Digite o texto e clique em **OK**. Funciona em uma célula que não tenha nota nem comentários.
- **Responder** adiciona o seu texto à discussão da célula selecionada.
- **Resolver Discussão** marca a discussão como concluída. O mesmo botão então aparece como **Reabrir Discussão**, que a abre novamente.
- **Excluir Comentário** remove as discussões das células selecionadas.
- **Comentário Anterior** e **Próximo Comentário** passam de uma célula com discussão para a seguinte.
- **Mostrar Comentários** abre o painel **Comentários** ao lado da planilha. Ele lista cada discussão da planilha como um cartão: a célula, cada comentário com o autor e a hora, e os botões **Responder**, **Resolver** ou **Reabrir**, e **Excluir**. **Ir Para** leva até a célula. O painel acompanha cada alteração, inclusive o Desfazer.
- **Converter Anotações em Comentários** transforma cada nota simples da planilha em uma discussão com um comentário. **Converter Comentários em Anotações** transforma cada discussão de volta em uma nota. Uma nota não tem respostas; por isso, os comentários de uma discussão se tornam uma única nota com todo o texto, e o JeffCrunch pede confirmação antes de fazer isso com uma discussão que tenha respostas.

Cada um desses itens é uma etapa que você pode desfazer.

Programas que não conseguem mostrar discussões exibem uma nota simples em seu lugar. O JeffCrunch mantém essa nota, com o texto da discussão, a cada vez que salva. Discussões, respostas e as pessoas que as escreveram permanecem como estavam quando você salva e abre a pasta de trabalho novamente. Uma discussão com mais de 500 comentários é exibida e mantida, mas o JeffCrunch não acrescenta nada a ela nem a altera.

Os comentários são salvos em uma pasta de trabalho .xlsx. Uma pasta de trabalho aberta a partir de um tipo de arquivo mais antigo, ou de um arquivo de texto, mantém os seus comentários somente depois que você a salva como pasta de trabalho .xlsx.

## Links

Um **link** leva você a algum lugar quando você o segue: a uma página da web, a um arquivo, a um endereço de email ou a um lugar nesta pasta de trabalho (uma célula, células ou um nome).

Para colocar um link em uma célula, selecione-a e use **Inserir > Link** (ou pressione **Ctrl+K**). Escolha para onde ele leva, digite o endereço ou o lugar (ou escolha um dos arquivos abertos recentemente em **Ou um arquivo recente**) e, se quiser, o texto que a célula deve mostrar e uma **dica de tela**. Em uma célula que já tem link, o mesmo comando aparece como **Editar Link**. **Remover Link** tira os links das células selecionadas. **Remover Hiperlinks** (no menu **Limpar** da guia Página Inicial, e no menu do botão direito de uma célula com link) tira os links e também a aparência que o link dá à célula, a cor azul e o sublinhado, para que ela deixe de parecer um link. Todo o resto da célula permanece: o preenchimento, o formato de número, as bordas, o alinhamento e a fonte própria. Uma célula que você deixou azul e sublinhada manualmente, sem o estilo Hiperlink, mantém essa aparência. Ambas as ações são uma etapa que você pode desfazer, e uma planilha protegida que não permite links as recusa. Os links e a aparência são salvos com a pasta de trabalho, em todos os formatos que têm links.

Aponte para uma célula com link para ver para onde ele leva. Para segui-lo, mantenha **Ctrl** pressionado e clique na célula, ou selecione a célula e use **Abrir Link** no menu do botão direito. Um clique simples apenas seleciona a célula.

O JeffCrunch nunca abre nada por conta própria. Uma página da web ou um endereço de email abre no seu programa habitual quando você segue o link. Um link para um arquivo, ou para outro tipo de endereço, pede sua confirmação antes. Um link que executaria um script nunca é aberto.

Um link acompanha a sua célula quando linhas ou colunas são inseridas ou excluídas e quando as células são classificadas, e é salvo com a pasta de trabalho.

## Estatísticas da Pasta de Trabalho

**Estatísticas da Pasta de Trabalho** (guia **Revisão**) conta o que a pasta de trabalho contém: quantas **Planilhas** ela tem, quantas **Células com dados** (células que contêm um número, um texto ou uma fórmula), quantas **Fórmulas**, quantos **Gráficos** e quantas **Tabelas dinâmicas**. Planilhas excluídas não são contadas. Os mesmos números estão na página **Informações** da guia Arquivo.

## Ortografia

**Ortografia** (guia **Revisão**, ou **F7**) verifica as palavras das suas células de texto, uma palavra incorreta de cada vez. Se você tiver selecionado mais de uma célula, verifica somente essas células. Caso contrário, verifica a planilha inteira. Não examina fórmulas, números nem datas.

A janela mostra a palavra que não está no dicionário, uma caixa chamada **Alterar para** com a melhor sugestão e uma lista de outras sugestões. Escolha uma sugestão ou digite a sua, e então escolha:

- **Alterar** coloca a nova palavra no lugar da errada naquela célula. Cada alteração é uma etapa que você pode desfazer.
- **Alterar Tudo** coloca a nova palavra no lugar da mesma palavra em todas as células verificadas, como uma única etapa que você pode desfazer.
- **Ignorar** deixa a palavra como está desta vez. **Ignorar Tudo** a deixa como está em todos os lugares até você fechar o JeffCrunch.
- **Adicionar** coloca a palavra no seu próprio dicionário, para que ela nunca mais seja marcada. Seu dicionário é compartilhado com os outros aplicativos do JeffOffice.

**Arquivo > Opções > Revisão de Texto** define se as palavras em letras maiúsculas e as palavras com números são verificadas. O JeffCrunch verifica no idioma em que é exibido, se tiver dicionário para ele, e em inglês americano caso contrário.

Enquanto você digita em uma célula, uma palavra que não está no dicionário recebe um sublinhado vermelho ondulado. Clique com o botão direito na palavra para escolher uma correção, ou para escolher **Ignorar Tudo** ou **Adicionar ao Dicionário**. As células da grade não são sublinhadas, e uma entrada que começa com sinal de igual nunca é verificada. Para ver, editar, importar ou exportar sua lista de palavras adicionadas, e para listar palavras que são sempre marcadas como incorretas, clique em **Arquivo > Opções > Revisão de Texto > Dicionário Personalizado**. Suas listas são as mesmas em todos os aplicativos do JeffOffice.

## Definir o idioma de algumas células

A verificação ortográfica examina cada célula no próprio idioma. Para alterá-lo, selecione as células e clique em **Definir Idioma de Revisão de Texto**, no grupo **Idioma** da guia **Revisão**. Escolha o idioma na lista e clique em **OK**. Uma célula que você não marcou é verificada no idioma definido em **Arquivo > Opções**. Uma planilha OpenDocument (.ods) mantém o idioma de cada célula. Uma pasta de trabalho .xlsx não tem onde guardá-lo, por isso o idioma é mantido apenas enquanto a pasta de trabalho estiver aberta.

## Não verificar algumas células

Alguns textos não devem ser verificados: um código, um nome ou uma linha em um idioma inventado. Selecione as células e clique em **Não Verificar Ortografia**, no grupo **Idioma**. **Ortografia** e os sublinhados vermelhos ondulados da célula as ignoram. Clique novamente para verificar as células mais uma vez. Um arquivo .ods mantém essa marca, e uma pasta de trabalho .xlsx a mantém apenas enquanto a pasta de trabalho estiver aberta.

## Traduzir texto

1. Selecione as células, ou selecione algumas das palavras que você está digitando em uma célula. O texto das células selecionadas é enviado uma linha por vez.
2. Clique em **Traduzir** no grupo **Idioma**.
3. Escolha o idioma para o qual traduzir.
4. Clique em **OK**.

Seu navegador da web então abre um serviço de tradução chamado LibreTranslate, com o seu texto nele. Nada é enviado até que você clique em **OK**. Você pode alterar o endereço do serviço na caixa, por exemplo para um que você mesmo executa. Somente endereços da web seguros (https) são usados, e endereços http deste computador. São enviadas no máximo 5.000 letras.

## Dicionário de sinônimos

**Dicionário de Sinônimos** (guia **Revisão**, ou **Shift+F7**) encontra palavras de sentido quase igual ao de uma palavra em uma célula. Ele procura a primeira palavra do texto da célula em que você está. Digite outra palavra em **Pesquisar** e pressione **Enter** para pesquisar essa em vez dela. Escolha uma palavra na lista e clique em **Substituir**, ou dê um clique duplo sobre ela. A palavra da célula é substituída, e uma primeira letra maiúscula continua maiúscula. **Pesquisar** procura a palavra escolhida, para seguir uma cadeia de significados. A lista de palavras funciona em inglês.

## Verificar Acessibilidade

**Verificar Acessibilidade** (guia **Revisão**) abre uma pequena janela que lista o que na pasta de trabalho é difícil de usar para alguns leitores, ou para um leitor de tela (um programa que lê a tela em voz alta). Cada linha começa com **Erro**, **Aviso** ou **Dica**. Clique em uma linha para ir até a célula.

Ele procura:

- Um gráfico ou imagem sem **texto alternativo** (as palavras que dizem o que ela mostra). Isto é um erro.
- Uma tabela sem linha de cabeçalho, de modo que suas colunas não têm nomes. Isto é um erro.
- **Células mescladas**, que são difíceis de percorrer com o teclado. Isto é um aviso.
- Texto difícil de ler contra a cor de fundo, como texto cinza-claro sobre branco. O JeffCrunch mede o quanto o texto se destaca do preenchimento. Texto pequeno precisa de uma razão de pelo menos 4,5 para 1, e texto grande (18 pontos, ou negrito de 14 pontos) precisa de 3 para 1. Ele indica a primeira célula de cada combinação de fonte e preenchimento. Isto é um aviso.
- Uma planilha que ainda tem o nome que recebeu, como Planilha1. Isto é uma dica.

Pressione **Verificar Novamente** depois de fazer alterações.
